#include "mii.h"

int readMiis(mii*dest){
    const char TARGET_PATH[] = "/shared2/menu/FaceLib/RFL_DB.dat";
    const char DAT_MAGIC[] = "RNOD";
    //ISFSの初期化
    int ret = ISFS_Initialize();
	int fd,fileSize,miiNum = 0,miiSlot = 0;
	char headBuf[5] = {};
	unsigned char *DatFileBuf; //RFL_DB.datを格納するバッファ
	printf("[*] Initializing ISFS subsystem...");
    if(ret == ISFS_OK){
		printf(" OK!\n");
        fd = ISFS_Open(TARGET_PATH,ISFS_OPEN_READ);
		if(fd > 0){
            fileSize = isfsGetFileSize(fd);
			if(fileSize == 0)return -1;
            DatFileBuf = allocate_memory(fileSize);//32バイト境界に載せる必要があるため、mallocは使用不可
            if(DatFileBuf == NULL){
                printf("Error:allocate_memory\n");
                return -1;
            }
            ISFS_Read(fd,DatFileBuf,fileSize);
			ISFS_Close(fd);
			ISFS_Deinitialize();
            memcpy(headBuf,DatFileBuf,4);
			if(strcmp(headBuf,DAT_MAGIC) != 0){
                printf("Error:magic doesn't match\n");
				free(DatFileBuf);
				return -1;
			}
            while(1){
				if(miiSlot >= MAX_MII_NUM)break;
			    memcpy((dest[miiNum]).rawData,DatFileBuf + 4 + miiSlot * MII_FILE_SIZE,MII_FILE_SIZE);
				miiSlot++;
				if(miiRawDataCheck((dest[miiNum]).rawData) != 0)continue;
                miiNum++;
			}
			free(DatFileBuf);
			allGetMiiInfo(dest,miiNum);
			return miiNum;
		}else{
            printf("Error:cannot open %s\n",TARGET_PATH);
		}
        ISFS_Deinitialize();
		return -1;
	}else{
		printf(" Error! (%d)\n",ret);
        return -1;
	}
}

int isfsGetFileSize(int fd){
	int error;
	if(fd < 1)return 0;
    fstats stats ATTRIBUTE_ALIGN(32);//ATTRIBUTE_ALIGN(32)で32バイト境界に載せる
	error = ISFS_GetFileStats(fd,&stats);
	if(error >= 0){
        return stats.file_length;
	}
	printf("Error:ISFS_GetFileStats (%d)\n",error);
	return 0;
}

int miiRawDataCheck(unsigned char*src){
    int i;
	for(i = 0;i < MII_FILE_SIZE;i++){
		if(src[i] != 0)return 0;
	}
	return -1;
}

int miiFileWrite(mii *Miis,int index,char *dir){
	char path[128] = {};
	FILE *f;
	sprintf(path,"%s/%08d.MII",dir,index + 1);
    f = fopen(path,"wb");
	if(!f){
		printf("\nError:fopen\n");
		return -1;
	}
	fwrite((Miis[index]).rawData,sizeof(unsigned char),MII_FILE_SIZE,f);
	fclose(f);
	return 0;
}

void*allocate_memory(unsigned int size){
    void*buf = memalign(32,(size+31)&(~31));
	memset(buf,0,(size+31)&(~31));
	return buf;
}

void getMiiInfo(mii *pmii){
	int i,tmpU16;
    for(i = 0;i < MII_NAME_LENGTH;i++){
		tmpU16 = (pmii->rawData[i * 2 + 2] << 8) + pmii->rawData[i * 2 + 3];
		if(tmpU16 > 127){
			pmii->name[i] = '?';
		}else if((tmpU16 < 0x20 && tmpU16 != 0) || tmpU16 == 127){
            pmii->name[i] = ' ';
		}else{
			pmii->name[i] = (char)tmpU16;
		}
	}
	pmii->name[MII_NAME_LENGTH] = 0;
	pmii->month = (pmii->rawData[0] >> 2) & 0xf;
	pmii->day = ((pmii->rawData[0] & 3) << 3) + (pmii->rawData[1] >> 5);
	pmii->favColor = (pmii->rawData[1] >> 1) & 0xf;
	return;
}

void allGetMiiInfo(mii*Miis,int num){
	int i;
    for(i = 0;i < num;i++){
		getMiiInfo(Miis + i);
	}
}

void showMiiTable(int index,int max,mii*Miis){
	char *showColor[] = {
		"\x1b[31m    red     \x1b[39m",
		"\x1b[33m   orange   \x1b[39m",
		"\x1b[33m   yellow   \x1b[39m",
		"\x1b[32myellow green\x1b[39m",
		"\x1b[32m   green    \x1b[39m",
		"\x1b[34m    blue    \x1b[39m",
        "\x1b[36m    cyan    \x1b[39m",
		"\x1b[35m    pink    \x1b[39m",
		"\x1b[35m   purple   \x1b[39m",
		"\x1b[37m    brown   \x1b[39m",
		"    white   ",
		"\x1b[7m    black   \x1b[0m",
		"   invalid  ",
		"   invalid  ",
		"   invalid  ",
		"   invalid  "
	};
	int page = index / SHOW_MII_NUM;
	int curIndex,i;
	mii* curMiiPtr;
    for(i = 0;i < SHOW_MII_NUM;i++){
        curIndex = i + page * SHOW_MII_NUM;
		curMiiPtr = Miis + curIndex;
        if(curIndex >= max)break;
		if(curIndex == index){
            printf("-->> ");
		}else{
			printf("     ");
		}
		if(curMiiPtr->month || curMiiPtr->day){
		    printf("%3d %10s %2d/%2d %s\n",curIndex + 1,curMiiPtr->name,curMiiPtr->month,curMiiPtr->day,showColor[curMiiPtr->favColor]);
		}else{
            printf("%3d %10s --/-- %s\n",curIndex + 1,curMiiPtr->name,showColor[curMiiPtr->favColor]);
		}
	}
}