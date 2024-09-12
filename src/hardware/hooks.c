#include <stdio.h>

#include <atmel_start.h>
#include <platform/hooks.h>
#include "sd_mmc/sd_mmc.h"
#include "sd_mmc/sd_mmc_protocol.h"

/* Printf Mutex */
SemaphoreHandle_t printf_mutex;
static StaticQueue_t printf_mutex_buf;

/* This is executed by the onehz task */
void hook_onehz(void) {
}

/* Reset your system to factory configuration here */
void hook_gndreset(void) {
}

/* This runs as a once-off task, the first time onehz starts */
void hook_init_task(void) {
}

/* This is at the beginning of the main routine, before usart and peripherals are inited */
void hook_init_early(void) {
#if defined(TEST_FAT_VMEM)
    init_vmem_fat_test_early();
#elif defined(TEST_FAT_SD)
    init_fat_sdmmc_test_early();
#endif
    // create as early as possible
    printf_mutex = xSemaphoreCreateMutexStatic(&printf_mutex_buf);
}

void hook_init(void) {

}

