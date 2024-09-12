#include "fat_sd_mmc.h"
#include "vmem/vmem.h"

typedef struct vmem_fat_file_driver_s {
    /** configuration **/

    int slot; /* The SD slot on which this file's FAT partition is located */
    int partition; /* The FAT Partition on which this file is located */
    const char * file_name; /* the name of the file, where this vmem will be stored. It will be created in the root of the file system, sub-directories are not supported currently */
    uint32_t max_size; /* the maximum size of the file (in bytes) before an error occurs */

    /** runtime values **/

    FF_Disk_t* disk; /* FAT driver disk handle */
    FF_FILE* file; /* FAT driver file handle */

} vmem_fat_file_driver_t;


void vmem_fat_file_init(const struct vmem_s* vmem);
void vmem_fat_file_read(const struct vmem_s* vmem, uint32_t addr, void * dataout, int len);
void vmem_fat_file_write(const struct vmem_s* vmem, uint32_t addr, void * datain, int len);

/*
 * we need to keep track of existing disk handles so that different vmem_fat_files which share a SD card can be properly synchronized.
 * to limit the amount of memory allocated for this internal house-keeping we limit the amount of concurrently supported SD slots and
 * partitions here.
 */
#define FAT_MAX_SD_SLOTS 2 /* sdmmc supports up to 255 slots, but 2 should be enough for most real-world use case */
#define FAT_MAX_PARTITIONS 4 /* the limit for non-extended partitions */

#define VMEM_DEFINE_FAT_FILE(slot_in, partition_in, file_name_in, name_in, size_in) \
	static vmem_fat_file_driver_t vmem_##name_in##_driver = { \
        .slot = slot_in,           \
        .partition = partition_in, \
        .file_name = file_name_in, \
        .max_size = size_in,       \
        .disk = NULL,           \
        .file = NULL,           \
	}; \
	__attribute__((section("vmem"))) \
	__attribute__((aligned(1))) \
	__attribute__((used)) \
	vmem_t vmem_##name_in = { \
		.type = VMEM_TYPE_DRIVER, \
		.name = #name_in, \
		.size = size_in, \
        .init = vmem_fat_file_init, \
		.read = vmem_fat_file_read, \
		.write = vmem_fat_file_write, \
		.driver = &vmem_##name_in##_driver, \
		.vaddr = NULL, /* initialized by init function */ \
	};

