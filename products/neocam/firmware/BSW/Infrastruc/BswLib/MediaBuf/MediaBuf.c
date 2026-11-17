#include "MediaBuf.h"
#include "MediaBuf_Backend.h"
#include "EbOs.h"

_Static_assert(MEDIA_BUF_SLOT_COUNT > 0U && MEDIA_BUF_SLOT_COUNT <= UINT16_MAX,
               "Media pool slot IDs must fit Neo_BufferTokenType");
_Static_assert(MEDIA_BUF_PREVIEW_SLOT_SIZE > 0U && MEDIA_BUF_PHOTO_SLOT_SIZE > 0U,
               "Media pool capacities must be nonzero");
_Static_assert(MEDIA_BUF_PREVIEW_SLOT_SIZE <= UINT32_MAX && MEDIA_BUF_PHOTO_SLOT_SIZE <= UINT32_MAX,
               "Media pool capacities must fit frame lengths");

typedef struct {
    uint8 *data;
    uint32 generation;
    uint32 references;
} MediaBuf_SlotType;

/* Generations survive deinit/reinit. An exhausted generation is never wrapped
 * to a previously issued token; that slot remains unavailable until reboot. */
static MediaBuf_SlotType slots[MEDIA_BUF_SLOT_COUNT];
static EbOs_MutexType pool_mutex;
static boolean initialized;

static uint32 slot_capacity(uint32 slot)
{
    return slot < MEDIA_BUF_PREVIEW_SLOT_COUNT ? MEDIA_BUF_PREVIEW_SLOT_SIZE : MEDIA_BUF_PHOTO_SLOT_SIZE;
}

static boolean valid_token(Neo_BufferTokenType token)
{
    return token.slot < MEDIA_BUF_SLOT_COUNT && token.generation != 0U &&
           slots[token.slot].references != 0U && slots[token.slot].generation == token.generation;
}

Neo_StatusType MediaBuf_Init(void)
{
    if (initialized) return NEO_OK;
    Neo_StatusType result = EbOs_MutexCreate(&pool_mutex);
    if (result != NEO_OK) return result;
    for (uint32 i = 0U; i < MEDIA_BUF_SLOT_COUNT; ++i) {
        slots[i].data = MediaBuf_BackendAllocate(slot_capacity(i));
        if (slots[i].data == NULL_PTR) {
            for (uint32 j = 0U; j < i; ++j) {
                MediaBuf_BackendFree(slots[j].data);
                slots[j].data = NULL_PTR;
            }
            EbOs_MutexDelete(pool_mutex);
            pool_mutex = NULL_PTR;
            return NEO_E_MEMORY;
        }
        slots[i].references = 0U;
    }
    initialized = TRUE;
    return NEO_OK;
}

Neo_StatusType MediaBuf_Deinit(void)
{
    if (!initialized) return NEO_OK;
    Neo_StatusType result = EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER);
    if (result != NEO_OK) return result;
    for (uint32 i = 0U; i < MEDIA_BUF_SLOT_COUNT; ++i) {
        if (slots[i].references != 0U) {
            EbOs_MutexUnlock(pool_mutex);
            return NEO_E_BUSY;
        }
    }
    initialized = FALSE;
    for (uint32 i = 0U; i < MEDIA_BUF_SLOT_COUNT; ++i) {
        MediaBuf_BackendFree(slots[i].data);
        slots[i].data = NULL_PTR;
    }
    EbOs_MutexUnlock(pool_mutex);
    EbOs_MutexDelete(pool_mutex);
    pool_mutex = NULL_PTR;
    return NEO_OK;
}

static Neo_StatusType acquire(uint32 first, uint32 count, uint8 **data, Neo_BufferTokenType *token)
{
    if (data == NULL_PTR || token == NULL_PTR) return NEO_E_PARAM;
    *data = NULL_PTR;
    if (!initialized) return NEO_E_NOT_READY;
    Neo_StatusType result = EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER);
    if (result != NEO_OK) return result;
    result = NEO_E_BUSY;
    for (uint32 i = first; i < first + count; ++i) {
        if (slots[i].references == 0U && slots[i].generation != UINT32_MAX) {
            ++slots[i].generation;
            slots[i].references = 1U;
            token->slot = (uint16)i;
            token->generation = slots[i].generation;
            *data = slots[i].data;
            result = NEO_OK;
            break;
        }
    }
    EbOs_MutexUnlock(pool_mutex);
    return result;
}

Neo_StatusType MediaBuf_Acquire(uint8 **data, Neo_BufferTokenType *token)
{
    return acquire(0U, MEDIA_BUF_PREVIEW_SLOT_COUNT, data, token);
}

Neo_StatusType MediaBuf_AcquirePhoto(uint8 **data, Neo_BufferTokenType *token)
{
    return acquire(MEDIA_BUF_PREVIEW_SLOT_COUNT, MEDIA_BUF_PHOTO_SLOT_COUNT, data, token);
}

Neo_StatusType MediaBuf_Retain(Neo_BufferTokenType token)
{
    if (token.slot >= MEDIA_BUF_SLOT_COUNT || token.generation == 0U) return NEO_E_PARAM;
    if (!initialized) return NEO_E_NOT_READY;
    Neo_StatusType result = EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER);
    if (result != NEO_OK) return result;
    if (!valid_token(token)) result = NEO_E_STATE;
    else if (slots[token.slot].references == UINT32_MAX) result = NEO_E_BUSY;
    else ++slots[token.slot].references;
    EbOs_MutexUnlock(pool_mutex);
    return result;
}

Neo_StatusType MediaBuf_Release(Neo_BufferTokenType token)
{
    if (token.slot >= MEDIA_BUF_SLOT_COUNT || token.generation == 0U) return NEO_E_PARAM;
    if (!initialized) return NEO_E_NOT_READY;
    Neo_StatusType result = EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER);
    if (result != NEO_OK) return result;
    if (!valid_token(token)) result = NEO_E_STATE;
    else --slots[token.slot].references;
    EbOs_MutexUnlock(pool_mutex);
    return result;
}

uint32 MediaBuf_Capacity(Neo_BufferTokenType token)
{
    if (!initialized || EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER) != NEO_OK) return 0U;
    uint32 capacity = valid_token(token) ? slot_capacity(token.slot) : 0U;
    EbOs_MutexUnlock(pool_mutex);
    return capacity;
}

uint32 MediaBuf_Outstanding(void)
{
    if (!initialized) return 0U;
    if (EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER) != NEO_OK) return MEDIA_BUF_SLOT_COUNT;
    uint32 count = 0U;
    for (uint32 i = 0U; i < MEDIA_BUF_SLOT_COUNT; ++i) count += slots[i].references != 0U ? 1U : 0U;
    EbOs_MutexUnlock(pool_mutex);
    return count;
}

static uint32 references(uint32 count)
{
    if (!initialized) return 0U;
    if (EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER) != NEO_OK) return UINT32_MAX;
    uint64 total = 0U;
    for (uint32 i = 0U; i < count; ++i) total += slots[i].references;
    EbOs_MutexUnlock(pool_mutex);
    return total > UINT32_MAX ? UINT32_MAX : (uint32)total;
}

uint32 MediaBuf_References(void) { return references(MEDIA_BUF_SLOT_COUNT); }
uint32 MediaBuf_PreviewReferences(void) { return references(MEDIA_BUF_PREVIEW_SLOT_COUNT); }
