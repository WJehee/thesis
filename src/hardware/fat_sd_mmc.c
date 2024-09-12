#include "drivers/fat_sd_mmc.h"
#include <sd_mmc.h>
#include <semphr.h>
#include "ff_sys.h"

/* Types */
typedef struct sd_info {
    uint8_t slot; /* the SD card slot this disk is associated with */
    card_type_t type; /* The SD card type as reported by sd_mmc_get_type */
    card_version_t version;
} sd_info_t;

/* Defines */
#define SDMMC_SIGNATURE 0x41404332ul
#define SECTOR_SIZE 512ul
#define IOMAN_MEM_SIZE 4096ul
#define SDMMC_DEBUG 0

/* SDMMC stack mutex */
static SemaphoreHandle_t sdmmc_mutex = NULL;

/*
 * Partition mutex
 */
static SemaphoreHandle_t sdmmc_part_mutex  = NULL;

/*
 * Read/Write functions
 */
static FF_Error_t fat_sdmmc_write(
        uint8_t *sourceBuf,
        uint32_t sectorStart,
        uint32_t sectorCount,
        FF_Disk_t *disk) {
#if SDMMC_DEBUG != 0
    FF_PRINTF("fat_sdmmc: > write @ %x (%lu secs)\n", sectorStart, sectorCount);
#endif
    if(disk == NULL || disk->ulSignature != SDMMC_SIGNATURE || disk->xStatus.bIsInitialised == pdFALSE) {
        return FF_ERR_DRIVER_FATAL_ERROR;
    }
    sd_info_t * info = (sd_info_t *) disk->pvTag;
    if(info == NULL) {
        return FF_ERR_DRIVER_FATAL_ERROR;
    }

    /* lock MCI mutex */
    xSemaphoreTake(sdmmc_mutex, portMAX_DELAY);

    if(sd_mmc_check(info->slot) != SD_MMC_OK) {
        FF_PRINTF("fat_sdmmc: write failed, non-ok status = %d for slot %d\n", sd_mmc_check(info->slot), info->slot);
        return FF_ERR_DRIVER_NOMEDIUM;
    }
    if(sectorStart >= disk->ulNumberOfSectors ||
            sectorStart + sectorCount >= disk->ulNumberOfSectors) {
        FF_PRINTF("fat_sdmmc: write of %lu sectors at %lu is out-of-bounds\n", sectorCount, sectorStart);
        return FF_ERR_IOMAN_OUT_OF_BOUNDS_READ | FF_ERRFLAG;
    }
    if(sectorCount > UINT16_MAX){
        FF_PRINTF("fat_sdmmc: can't write more than UINT16_MAX blocks\n");
        return FF_ERR_DRIVER_FATAL_ERROR;
    }

    sd_mmc_err_t err;
    err = sd_mmc_init_write_blocks(info->slot, sectorStart, (uint16_t) sectorCount);
    if(err != SD_MMC_OK) {
        FF_PRINTF("fat_sdmmc: error during sd_mmc_init_write_blocks: %d\n", err);
        return FF_ERR_DRIVER_FATAL_ERROR;
    }
    err = sd_mmc_start_write_blocks(sourceBuf, (uint16_t) sectorCount);
    if(err != SD_MMC_OK) {
        FF_PRINTF("fat_sdmmc: error during sd_mmc_start_write_blocks: %d\n", err);
        return FF_ERR_DRIVER_FATAL_ERROR;
    }
    err = sd_mmc_wait_end_of_write_blocks(false);
    if(err != SD_MMC_OK) {
        FF_PRINTF("fat_sdmmc: error during sd_mmc_wait_end_of_write_blocks: %d\n", err);
        return FF_ERR_DRIVER_FATAL_ERROR;
    }

    /* unlock MCI mutex */
    xSemaphoreGive(sdmmc_mutex);

#if SDMMC_DEBUG != 0
	FF_PRINTF("fat_sdmmc: < write\n");
#endif
    return FF_ERR_NONE;
}

static FF_Error_t fat_sdmmc_read(
        uint8_t *destinationBuf,
        uint32_t sectorStart,
        uint32_t sectorCount,
        FF_Disk_t * disk) {

#if SDMMC_DEBUG != 0
	FF_PRINTF("fat_sdmmc: > read @ %x (%lu secs)\n", sectorStart, sectorCount);
#endif
    if(disk == NULL || disk->ulSignature != SDMMC_SIGNATURE || disk->xStatus.bIsInitialised == pdFALSE) {
        return FF_ERR_DRIVER_FATAL_ERROR;
    }
    sd_info_t * info = (sd_info_t *) disk->pvTag;
    if(info == NULL) {
        return FF_ERR_DRIVER_FATAL_ERROR;
    }

    /* lock MCI mutex */
    xSemaphoreTake(sdmmc_mutex, portMAX_DELAY);

    if(sd_mmc_check(info->slot) != SD_MMC_OK) {
        FF_PRINTF("fat_sdmmc: read failed, non-ok status = %d for slot %d\n", sd_mmc_check(info->slot), info->slot);
        return FF_ERR_DRIVER_NOMEDIUM;
    }
    if(sectorStart >= disk->ulNumberOfSectors ||
            sectorStart + sectorCount >= disk->ulNumberOfSectors) {
        FF_PRINTF("fat_sdmmc: read of %lu sectors at %lu is out-of-bounds\n", sectorCount, sectorStart);
        return FF_ERR_IOMAN_OUT_OF_BOUNDS_READ | FF_ERRFLAG;
    }
    if(sectorCount > UINT16_MAX){
        FF_PRINTF("fat_sdmmc: can't read more than UINT16_MAX blocks\n");
        return FF_ERR_DRIVER_FATAL_ERROR;
    }

    sd_mmc_err_t err;
    err = sd_mmc_init_read_blocks(info->slot, sectorStart, (uint16_t) sectorCount);
    if(err != SD_MMC_OK) {
        FF_PRINTF("fat_sdmmc: error during sd_mmc_init_read_blocks: %d\n", err);
        return FF_ERR_DRIVER_FATAL_ERROR;
    }
    err = sd_mmc_start_read_blocks(destinationBuf, (uint16_t) sectorCount);
    if(err != SD_MMC_OK) {
        FF_PRINTF("fat_sdmmc: error during sd_mmc_start_read_blocks: %d\n", err);
        return FF_ERR_DRIVER_FATAL_ERROR;
    }
    err = sd_mmc_wait_end_of_read_blocks(false);
    if(err != SD_MMC_OK) {
        FF_PRINTF("fat_sdmmc: error during sd_mmc_wait_end_of_read_blocks: %d\n", err);
        return FF_ERR_DRIVER_FATAL_ERROR;
    }

    /* unlock MCI mutex */
    xSemaphoreGive(sdmmc_mutex);

#if SDMMC_DEBUG != 0
	FF_PRINTF("fat_sdmmc: < read\n", sectorStart, sectorCount);
#endif
    return FF_ERR_NONE;
}

/* inits the stack */
void fat_sdmmc_init() {
    /* NOTE: this should call sd_mmc_stack_init(), but that is done by atmel_start_init(); */

    /* create mutex to guard access between different IOManager */
    /* XXX: this in an unfortunate side effect of the fact that a different IOManager is recreated for each FF_Disk_t
     *      structure and is bound to it. For the used sd_mmc stack a global IOManager is needed as to synchronize
     *      between the shared MCI interface.
     *      This global mutex emulates this behaviour even when multiple IOManager are created & used concurrently
     */
    static StaticSemaphore_t sdmmc_mutex_buffer;
    if(sdmmc_mutex == NULL) {
        sdmmc_mutex = xSemaphoreCreateRecursiveMutexStatic(&sdmmc_mutex_buffer);
        configASSERT(sdmmc_mutex);
    }
}

/* inits and mounts the requested SD-MMC card partition at the mount path */
FF_Disk_t * fat_sdmmc_init_slot(uint8_t slot) {
    /* lock MCI mutex */
    xSemaphoreTake(sdmmc_mutex, portMAX_DELAY);

    /* check slot validity */
    uint8_t num_slots = sd_mmc_nb_slot();
    if(slot >= num_slots) {
        FF_PRINTF("fat_sdmmc: invalid slot %d\n", slot);
        return NULL;
    }
    /* check SD card in specified slot */
    sd_mmc_err_t state = sd_mmc_check(slot);
    if(state != SD_MMC_OK && state != SD_MMC_INIT_ONGOING) {
        FF_PRINTF("fat_sdmmc: error with SD card in slot %d: state=%d\n", slot, state);
        return NULL;
    }
    /* retrieve information from SD card */
    card_type_t card_type = sd_mmc_get_type(slot);
    card_version_t card_version = sd_mmc_get_version(slot);
    FF_PRINTF("fat_sdmmc: init card type=%d, version=%2x\n", card_type, card_version);
    uint32_t card_size = sd_mmc_get_capacity(slot);
    FF_PRINTF("fat_sdmmc: card capacity %lu KiB\n", card_size);
    bool card_protected = sd_mmc_is_write_protected(slot);
    if(card_protected) {
        FF_PRINTF("fat_sdmmc: WARNING WARNING card in slot %d is write-protected\n", slot);
    }
    /* unlock MCI mutex */
    xSemaphoreGive(sdmmc_mutex);

    /* move to sd_info_t structure */
    sd_info_t * info = (sd_info_t *) pvPortMalloc(sizeof(sd_info_t));
    if(info == NULL) {
        FF_PRINTF("fat_sdmmc: malloc of sd_info_t failed!\n");
        return NULL;
    }
    info->slot = slot;
    info->type = card_type;
    info->version = card_version;

    /* init FF_Disk_t structure */
    FF_Disk_t * disk = (FF_Disk_t *) pvPortMalloc(sizeof (FF_Disk_t));
    if(disk == NULL) {
        FF_PRINTF("fat_sdmmc: malloc of FF_Disk_t failed!\n");
        return NULL;
    }
    /* set to zero for future compatibility */
    memset(disk, '\0', sizeof(*disk));

    /* update pvTag to point to custom info structure */
    disk->pvTag = (void *) info;

    /* create mutex */
    if(sdmmc_part_mutex == NULL) {
        sdmmc_part_mutex = xSemaphoreCreateRecursiveMutex();
        if(sdmmc_part_mutex == NULL) {
            FF_PRINTF("fat_sdmmc: mutex creation failed!\n");
            return NULL;
        }
    }
    disk->ulSignature = SDMMC_SIGNATURE;
    /* The ATMEL MMC drivers returns the capacity in KB therefore we need to
     * multiply it by 2 to get the numbers of sectors if the sector size is 512,
     * otherwise we must do the slow calculation. */
    uint32_t num_sectors;
    if(SECTOR_SIZE == 512ul) {
        num_sectors = card_size << 1;
    } else {
        num_sectors = card_size*1024ul / SECTOR_SIZE;
    }
    disk->ulNumberOfSectors = num_sectors;

    /* create parameters for IOMan initialization */
    FF_CreationParameters_t creation_params;
    memset(&creation_params, '\0', sizeof(creation_params));
    /* XXX: this is taken from ATSAM4E/ff_sddisk_r.c without knowing what its purpose is  */
    creation_params.ulMemorySize = IOMAN_MEM_SIZE;
    creation_params.ulSectorSize = SECTOR_SIZE;
    creation_params.fnWriteBlocks = fat_sdmmc_write;
    creation_params.fnReadBlocks = fat_sdmmc_read;
    creation_params.pxDisk = disk;
    /* due to the underlying use of MCI.Sync this device cannot be reentrant */
    creation_params.xBlockDeviceIsReentrant = pdFALSE;
    creation_params.pvSemaphore = (void *) sdmmc_part_mutex;

    FF_Error_t err;
    disk->pxIOManager = FF_CreateIOManger(&creation_params, &err);
    if(disk->pxIOManager == NULL) {
        FF_PRINTF("fat_sdmmc: failed to create IOManager with error: %s\n", (const char*) FF_GetErrMessage(err));
        fat_sdmmc_deinit(disk);
        return NULL;
    }
    /* update disk structure with metainformation */
    disk->xStatus.bIsInitialised = pdTRUE;
    return disk;
}

FF_Error_t fat_sdmmc_mount(FF_Disk_t* disk, int partition, char * mount_path) {
    if(disk == NULL || disk->xStatus.bIsInitialised == pdFALSE || disk->pxIOManager == NULL) {
        return FF_ERR_DRIVER_FATAL_ERROR;
    }

    disk->xStatus.bPartitionNumber = partition;
    /* mount the disk */
    FF_Error_t err = FF_Mount(disk, partition);
    if(FF_isERR(err)) {
        FF_PRINTF("fat_sdmmc: failed to mount partition %d  error: %s\n", partition, (const char*) FF_GetErrMessage(err));
        /* try to format the disk in this case */
        err = fat_sdmmc_format(disk);
        if(FF_isERR(err)) {
            FF_PRINTF("fat_sdmmc: failed to format\n") ;
            return err;
        }
        /* and try mounting again */
        err = FF_Mount(disk, partition);
        if(FF_isERR(err)) {
            FF_PRINTF("fat_sdmmc: failed to mount a second time. giving up!\n");
            return err;
        }
    }
    disk->xStatus.bIsMounted = pdTRUE;
    FF_FS_Add(mount_path, disk);
    FF_PRINTF("fat_sdmmc: mounted partition %d @ %s", partition, mount_path);
    return FF_ERR_NONE;
}

FF_Error_t fat_sdmmc_deinit(FF_Disk_t* disk) {
    if(disk != NULL) {
        disk->ulSignature = 0;
        disk->xStatus.bIsInitialised = 0;
        if(disk->pxIOManager != NULL) {
            FF_Error_t err;
            if(FF_Mounted(disk->pxIOManager)) {
                err = FF_Unmount(disk);
                if(FF_isERR(err)) {
                    return err;
                }
            }
            err = FF_DeleteIOManager(disk->pxIOManager);
            if(FF_isERR(err)) {
                return err;
            }
        }
        vPortFree(disk);
    }
    return FF_ERR_NONE;
}

FF_Error_t fat_sdmmc_format(FF_Disk_t* disk) {
    FF_Error_t err;
    /* partition disk */
    FF_PRINTF("partitioning disk...");
    FF_PartitionParameters_t parms;
    memset(&parms, '\0', sizeof(parms));

    parms.ulSectorCount = disk->ulNumberOfSectors;
    parms.ulHiddenSectors = 1;
    /* a single partition taking 4 percent of the space */
    parms.xSizes[0] = 4;
    parms.eSizeType = eSizeIsPercent;

    err = FF_Partition(disk, &parms);
    if(FF_isERR(err)) {
        FF_PRINTF("partitioning failed. error: %s", (const char*) FF_GetErrMessage(err));
        return err;
    }
    FF_PRINTF("partitioning successful");

    /* format first partition */
    FF_PRINTF("formatting first partition...");
    err = FF_Format(disk, 0, pdFALSE, pdFALSE);
    if(FF_isERR(err)) {
        FF_PRINTF("formatting failed. error: %s", (const char*) FF_GetErrMessage(err));
        return err;
    }
    return FF_ERR_NONE;
}

