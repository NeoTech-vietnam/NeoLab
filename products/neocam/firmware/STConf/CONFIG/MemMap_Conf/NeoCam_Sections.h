#ifndef NEOCAM_SECTIONS_H
#define NEOCAM_SECTIONS_H
/* ESP-IDF's linker scripts place .rodata.* in flash and .bss.* in internal DRAM.
 * The host runner preserves section names for compile/link-map checks. */
#define NEOCAM_CONST_SECTION __attribute__((section(".rodata.neocam")))
#define NEOCAM_ZERO_SECTION __attribute__((section(".bss.neocam")))
#endif
