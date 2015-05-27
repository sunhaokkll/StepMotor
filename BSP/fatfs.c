#include "includes.h"

/*******************************************************************************
  * @函数名称	scan_files
  * @函数说明   搜索文件目录下所有文件 
  * @输入参数   path: 根目录 
  * @输出参数   无
  * @返回参数   FRESULT
  * @注意事项	无
  *****************************************************************************/
//FRESULT scan_files (
//    char* path        /* Start node to be scanned (also used as work area) */
//)
//{
//    FRESULT res;
//    FILINFO fno;
//    DIR dir;
//    int i;
//    char *fn;   /* This function is assuming non-Unicode cfg. */
//#if _USE_LFN
//    static char lfn[_MAX_LFN + 1];
//    fno.lfname = lfn;
//    fno.lfsize = sizeof(lfn);
//#endif


//    res = f_opendir(&dir, path);                       /* Open the directory */
//    if (res == FR_OK) {
//        i = strlen(path);
//        for (;;) {
//            res = f_readdir(&dir, &fno);                   /* Read a directory item */
//            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
//            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
//#if _USE_LFN
//            fn = *fno.lfname ? fno.lfname : fno.fname;
//#else
//            fn = fno.fname;
//#endif
//            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
//                sprintf(&path[i], "/%s", fn);
//				printf("scan file - %s\n\r",path);
//                res = scan_files(path);
//                if (res != FR_OK) break;
//                path[i] = 0;
//            } else {                                       /* It is a file. */
//                printf("scan file - %s/%s\n\r", path, fn);
//            }
//        }
//    }else{
//		printf("scan files error : %d\n\r",res);
//	}

//    return res;
//}
///*******************************************************************************
//  * @函数名称	SD_TotalSize
//  * @函数说明   文件空间占用情况 
//  * @输入参数   无 
//  * @输出参数   无
//  * @返回参数   1: 成功 
//  				0: 失败
//  * @注意事项	无
//  *****************************************************************************/
//int SD_TotalSize(char *path)
//{
//    FATFS *fs;
//    DWORD fre_clust;        
//    FRESULT res;
//    res = f_getfree(path, &fre_clust, &fs);  /* 必须是根目录，选择磁盘0 */
//    if ( res==FR_OK ) 
//    {
//	  printf("\n\rget %s drive space.\n\r",path);
//	  /* Print free space in unit of MB (assuming 512 bytes/sector) */
//      printf("%d MB total drive space.\r\n"
//           "%d MB available.\r\n",
//           ( (fs->n_fatent - 2) * fs->csize ) / 2 /1024 , (fre_clust * fs->csize) / 2 /1024 );
//		
//	  return 1;
//	}
//	else
//	{ 
//	  printf("\n\rGet total drive space faild!\n\r");
//	  return 0;   
//	}
//}
