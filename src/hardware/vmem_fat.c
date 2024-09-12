#include "drivers/vmem_fat.h"

#include "FreeRTOS.h"
#include <stdio.h>
#include "ff_headers.h"
#include "ff_stdio.h"
#include "debug.h"

static FF_Disk_t * vmem_fat_disk_cache[FAT_MAX_SD_SLOTS][FAT_MAX_PARTITIONS] = { NULL };

FF_Disk_t* vmem_fat_get_disk_or_init(int slot, int partition) {
    configASSERT(slot >= 0 && slot < FAT_MAX_SD_SLOTS);
    configASSERT(partition >= 0 && partition < FAT_MAX_PARTITIONS);

    if(vmem_fat_disk_cache[slot][partition] != NULL) {
        DLOG("found existing disk!");
        return vmem_fat_disk_cache[slot][partition];
    }

    /* open disk */
    DLOG("initing disk for slot %d...", slot);
    FF_Disk_t * disk = fat_sdmmc_init_slot(slot);
    if(disk == NULL) {
        DLOG("disk init failed!");
        return NULL;
    }
    DLOG("disk inited");

    /* generate mount path */
    char buf[25]; /* arbitrary length here */
    snprintf(buf, sizeof(buf), "/%d/%d", slot, partition);

    /* mount partition */
    DLOG("mounting partition %d of slot %d @ %s", partition, slot, buf);
    FF_Error_t err = fat_sdmmc_mount(disk, partition, buf);
    if(FF_isERR(err)) {
        DLOG("mounting failed. error: %s", (const char*) FF_GetErrMessage(err));
        return NULL;
    }
    DLOG("mounted successfully.");

    /* save in cache */
    vmem_fat_disk_cache[slot][partition] = disk;

    return disk;
}


void vmem_fat_file_init(const struct vmem_s * vmem) {
    vmem_fat_file_driver_t* drv = (vmem_fat_file_driver_t*) vmem->driver;

    /* get or create disk */
    DLOG("creating disk");
    FF_Disk_t * disk = vmem_fat_get_disk_or_init(drv->slot, drv->partition);
    drv->disk = disk;
    DLOG("created disk successfully");

    return; /* success */
}

FF_FILE* vmem_fat_file_open_or_create(vmem_fat_file_driver_t* drv, const char * mode) {
    configASSERT(drv->file_name);
    configASSERT(strlen(drv->file_name) < 20); /* long filename might overflow the stack here */

    char buf[10 + strlen(drv->file_name)]; /* 10 for slot + partition */
    snprintf(buf, sizeof(buf), "/%d/%d/%s", drv->slot, drv->partition, drv->file_name);

    FF_FILE * file = ff_fopen(buf, mode);
    int err = stdioGET_ERRNO();
    if(err == pdFREERTOS_ERRNO_ENOENT || (err == pdFREERTOS_ERRNO_NONE && ff_filelength(file) < drv->max_size)) {
        ff_fclose(file);
        DLOG("file %s not found or invalid: trying to create it..", buf);
        FF_FILE * tmp_file = ff_fopen(buf, "w");
        configASSERT(stdioGET_ERRNO() == pdFREERTOS_ERRNO_NONE);
        ff_fclose(tmp_file);

        DLOG("file created, trying to truncate it to %lu", drv->max_size);
        tmp_file = ff_truncate(buf, drv->max_size);
        configASSERT(stdioGET_ERRNO() == pdFREERTOS_ERRNO_NONE && tmp_file != NULL);
        ff_fclose(tmp_file);
        DLOG("file truncated, opening again in mode %s for further usage", mode);

        file = ff_fopen(buf, mode);
        configASSERT(stdioGET_ERRNO() == pdFREERTOS_ERRNO_NONE);
    }
    else if(err != pdFREERTOS_ERRNO_NONE) {
        DLOG("couldn't open file %s : %s (%d)\n", drv->file_name, strerror(err), err);
        configASSERT(0);
        return NULL;
    }
    return file;

}

void vmem_fat_file_read(const struct vmem_s* vmem, uint32_t addr, void * dataout, int len) {
    vmem_fat_file_driver_t* drv = (vmem_fat_file_driver_t*) vmem->driver;

    configASSERT(addr < drv->max_size);
    if(addr >= drv->max_size) {
        DLOG("read out-of-bounds for file: %s\n (@%p)", ((vmem_fat_file_driver_t*) vmem->driver)->file_name, (void*) addr);
        return;
    }

    DLOG("read @ %p, len=%d", (void *) addr, len);

    FF_FILE* file = vmem_fat_file_open_or_create(drv, "r");
    configASSERT(file);

    ff_fseek(file, addr, FF_SEEK_SET);
    configASSERT(stdioGET_ERRNO() == pdFREERTOS_ERRNO_NONE);

    ff_fread(dataout, len, 1, file);
    configASSERT(stdioGET_ERRNO() == pdFREERTOS_ERRNO_NONE);

    ff_fclose(file);
}

void vmem_fat_file_write(const struct vmem_s* vmem, uint32_t addr, void * datain, int len) {
    vmem_fat_file_driver_t* drv = (vmem_fat_file_driver_t*) vmem->driver;
    configASSERT(addr < drv->max_size);
    if(addr >= drv->max_size) {
        DLOG("write out-of-bounds for file: %s\n (@%p)", ((vmem_fat_file_driver_t*) vmem->driver)->file_name, (void*) addr);
        return;
    }

    DLOG("write @ %p len=%d", (void *) addr ,len);

    FF_FILE* file = vmem_fat_file_open_or_create(drv, "r+");
    configASSERT(file);

    ff_fseek(file, addr, FF_SEEK_SET);
    configASSERT(stdioGET_ERRNO() == pdFREERTOS_ERRNO_NONE);

    ff_fwrite(datain, len, 1, file);
    configASSERT(stdioGET_ERRNO() == pdFREERTOS_ERRNO_NONE);

    ff_fclose(file);
}

