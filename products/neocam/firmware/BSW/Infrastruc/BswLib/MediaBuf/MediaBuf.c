#include "MediaBuf.h"
#include "MediaBuf_Backend.h"
#include "EbOs.h"

_Static_assert(MEDIA_BUF_SLOT_COUNT > 0U && MEDIA_BUF_SLOT_COUNT <= UINT16_MAX,
               "Media pool slot IDs must fit Neo_BufferTokenType");
_Static_assert(MEDIA_BUF_SLOT_SIZE > 0U && MEDIA_BUF_SLOT_SIZE <= UINT32_MAX,
               "Media pool capacity must fit a frame length");

typedef struct {
    uint8 *data;
    uint32 generation;
    boolean used;
} MediaBuf_SlotType;

/* Generations intentionally survive deinit/reinit. They never wrap to a
 * previously issued token: an exhausted slot remains unavailable. */
static MediaBuf_SlotType slots[MEDIA_BUF_SLOT_COUNT];
static EbOs_MutexType pool_mutex;
static boolean initialized;

Neo_StatusType MediaBuf_Init(void)
{
    if (initialized) {
        return NEO_OK;
    }
    Neo_StatusType result = EbOs_MutexCreate(&pool_mutex);
    if (result != NEO_OK) {
        return result;
    }
    for (uint32 i = 0U; i < MEDIA_BUF_SLOT_COUNT; ++i) {
        slots[i].data = MediaBuf_BackendAllocate(MEDIA_BUF_SLOT_SIZE);
        if (slots[i].data == NULL_PTR) {
            for (uint32 j = 0U; j < i; ++j) {
                MediaBuf_BackendFree(slots[j].data);
                slots[j].data = NULL_PTR;
            }
            EbOs_MutexDelete(pool_mutex);
            pool_mutex = NULL_PTR;
            return NEO_E_MEMORY;
        }
        slots[i].used = FALSE;
    }
    initialized = TRUE;
    return NEO_OK;
}

Neo_StatusType MediaBuf_Deinit(void)
{
    if (!initialized) {
        return NEO_OK;
    }
    Neo_StatusType result = EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER);
    if (result != NEO_OK) {
        return result;
    }
    for (uint32 i = 0U; i < MEDIA_BUF_SLOT_COUNT; ++i) {
        if (slots[i].used) {
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

Neo_StatusType MediaBuf_Acquire(uint8 **data, Neo_BufferTokenType *token)
{
    if (data == NULL_PTR || token == NULL_PTR) {
        return NEO_E_PARAM;
    }
    *data = NULL_PTR;
    if (!initialized) {
        return NEO_E_NOT_READY;
    }
    Neo_StatusType result = EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER);
    if (result != NEO_OK) {
        return result;
    }
    result = NEO_E_BUSY;
    for (uint32 i = 0U; i < MEDIA_BUF_SLOT_COUNT; ++i) {
        if (!slots[i].used && slots[i].generation != UINT32_MAX) {
            ++slots[i].generation;
            slots[i].used = TRUE;
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

Neo_StatusType MediaBuf_Release(Neo_BufferTokenType token)
{
    if (token.slot >= MEDIA_BUF_SLOT_COUNT || token.generation == 0U) {
        return NEO_E_PARAM;
    }
    if (!initialized) {
        return NEO_E_NOT_READY;
    }
    Neo_StatusType result = EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER);
    if (result != NEO_OK) {
        return result;
    }
    MediaBuf_SlotType *slot = &slots[token.slot];
    if (!slot->used || slot->generation != token.generation) {
        result = NEO_E_STATE;
    } else {
        slot->used = FALSE;
    }
    EbOs_MutexUnlock(pool_mutex);
    return result;
}

uint32 MediaBuf_Outstanding(void)
{
    if (!initialized) {
        return 0U;
    }
    if (EbOs_MutexLock(pool_mutex, EBOS_WAIT_FOREVER) != NEO_OK) {
        /* Conservatively prevent cleanup if synchronization ever fails. */
        return MEDIA_BUF_SLOT_COUNT;
    }
    uint32 count = 0U;
    for (uint32 i = 0U; i < MEDIA_BUF_SLOT_COUNT; ++i) {
        count += slots[i].used ? 1U : 0U;
    }
    EbOs_MutexUnlock(pool_mutex);
    return count;
}
