#pragma once

#include "ff_headers.h"

/**
 * \brief Initializes the FAT SDMMC stack. Needs to be called before any initialization of specific slots and/or access.
 * /
 */
void fat_sdmmc_init();

/**
 * \brief Initializes and registers a SD card slot with the FreeRTOS+FAT subsystem.
 *
 * \param[in] slot The SD card slot that should be initialized
 * \return return a FFDisk structure if the card is valid and present and the initialization was succesful, otherwise NULL.
 */
FF_Disk_t * fat_sdmmc_init_slot(uint8_t slot);

/**
 * \brief Deregisters a SD card slot with the FreeRTOS+FAT subsystem.
 *
 * \param[in] disk The FF_Disk_t structure corresponding to the SD card slot
 * \return A FF_Error_t value to indicate whether or not the operation was successful.
 */
FF_Error_t fat_sdmmc_deinit(FF_Disk_t * disk);


/**
 * \brief Mount a FAT partition of a previously registered SD card with the FreeRTOS+FAT subsystem.
 *
 * \param[in] disk The FF_Disk_t structure that was initialized through \ref fat_sdmmc_init_slot().
 * \param[in] partition The partition of the card that should be mounted
 * \param[in] mount_path The path at which the mounted partition will be available afterwards.
 * \return A FF_Error_t value to indicate whether or not the operation was successful.
 */
FF_Error_t fat_sdmmc_mount(FF_Disk_t * disk, int partition, char * mount_path);

/**
 * \brief Format and partitions the SDMMC devices with a single partition of fixed size (currently at 4 percent of total capacity)
 *
 * \param[in] disk The FF_Disk_t structure that was initialized through \ref fat_sdmmc_init_slot().
 * \return A FF_Error_t value to indicate whether or not the operation was successful
 */
FF_Error_t fat_sdmmc_format(FF_Disk_t * disk);

