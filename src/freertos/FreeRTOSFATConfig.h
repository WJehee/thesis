#include <FreeRTOS_errno_FAT.h>

/*
These are the settings for the FreeRTOS+FAT library
see FreeRTOSFATConfigDefaults.h for more information
on each parameter.
*/


#define ffconfigBYTE_ORDER                          pdFREERTOS_LITTLE_ENDIAN
#define ffconfigHAS_CWD                             1
#define ffconfigCWD_THREAD_LOCAL_INDEX              0
#define ffconfigLFN_SUPPORT                         1
#define ffconfigINCLUDE_SHORT_NAME                  1
#define ffconfigSHORTNAME_CASE                      1
#define ipconfigQUICK_SHORT_FILENAME_CREATION       0
#define ffconfigUNICODE_UTF16_SUPPORT               0
#define ffconfigUNICODE_UTF8_SUPPORT                0
#define ffconfigFAT12_SUPPORT                       0
#define ffconfigOPTIMISE_UNALIGNED_ACCESS           1
#define ffconfigCACHE_WRITE_THROUGH                 1
#define ffconfigWRITE_BOTH_FATS                     1
#define ffconfigWRITE_FREE_COUNT                    1
#define ffconfigTIME_SUPPORT                        0
#define ffconfigREMOVABLE_MEDIA                     0
#define ffconfigMOUNT_FIND_TREE                     1
#define ffconfigFSINFO_TRUSTED                      0 /* ???, unclear in documentation */
#define ffconfigFINDAPI_ALLOW_WILDCARDS             0
#define ffconfigWILDCARD_CASE_INSENSITIVE           0
#define ffconfigPATH_CACHE                          1
#define ffconfigPATH_CACHE_DEPTH                    5
#define ffconfigHASH_CACHE                          0
#define ffconfigHASH_FUNCTION                       CRC16
#define ffconfigMKDIR_RECURSIVE                     0
#define ffconfigMALLOC( size )                      pvPortMalloc( size )
#define ffconfigFREE( ptr )                         vPortFree( ptr )
#define ffconfig64_NUM_SUPPORT                      0
#define ffconfigMAX_PARTITIONS                      4
#define ffconfigMAX_FILE_SYS                        4
#define ffconfigDRIVER_BUSY_SLEEP_MS                20
#define ffconfigFPRINTF_SUPPORT                     0
#define ffconfigFPRINTF_BUFFER_LENGTH               128
#define ffconfigDEBUG                               1
#if ffconfigDEBUG == 1
#define FF_PRINTF(fmt,...)                           printf("[fatdrv] " fmt, ##__VA_ARGS__);
#else
#define FF_PRINTF(fmt,...)
#endif
#define ffconfigLONG_ERR_MSG                        0
#define ffconfigHAS_FUNCTION_TAB                    0
#define ffconfigMIRROR_FATS_UMOUNT                  0 /* ???, unclear in documentation */
#define ffconfigFAT_CHECK                           0
#define ffconfigMAX_FILENAME                        129
#define ffconfigUSE_DELTREE                         1
#define ffconfigFILE_EXTEND_FLUSHES_BUFFERS         1

