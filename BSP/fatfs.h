#ifndef __FATFS_H__
#define __FATFS_H__
#include "includes.h"

FRESULT scan_files ( char* path);
int SD_TotalSize(char *path);
#endif
