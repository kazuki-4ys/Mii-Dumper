#ifndef _MII_H_
#define _MII_H_
#define MII_FILE_SIZE 0x4A
#define MAX_MII_NUM 100
#define MII_NAME_LENGTH 10

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

typedef struct {
	unsigned char rawData[MII_FILE_SIZE];
    char name[MII_NAME_LENGTH + 1];
	unsigned int month;
	unsigned int day;
	unsigned int favColor;
} mii;

int readMiis(mii*);
int isfsGetFileSize(int);
int miiRawDataCheck(unsigned char*);
int miiFileWrite(mii*,int,char*);
void*allocate_memory(unsigned int);
void getMiiInfo(mii*);
void allGetMiiInfo(mii*,int);

#endif //_MII_H_