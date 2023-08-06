#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <dirent.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>
#include "mii.h"

#define FOOTER_LINE_1 "A / Dump Mii | UP DOWN LEFT RIGHT / Select Mii\n"
#define FOOTER_LINE_2 "HOME / Exit\n"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

int sdInitialize = 0;

int SD_Initialize(){
	int ret = fatMountSimple("sd", &__io_wiisd);
	if(!ret) return ret;
	return 1;
}

void SD_Deinitialize(){
	fatUnmount("sd:/");
    __io_wiisd.shutdown();
}

void updateMiiList(int index,int max,mii *Miis){
	int i;
    printf("\x1b[9;0H");
	for(i = 0;i < SHOW_MII_NUM;i++){
        printf("                                                                      \n");
	}
	printf("\x1b[9;0H");
	showMiiTable(index,max,Miis);
	printf("\x1b[%d;0H",SHOW_MII_NUM + 10);
	for(i = 0;i < 2;i++){
        printf("                                                                      \n");
	}
	printf("\x1b[%d;0H",SHOW_MII_NUM + 10);
	printf(FOOTER_LINE_1);
	printf(FOOTER_LINE_2);
    return;
}

void appExit(){
    if(sdInitialize){
        SD_Deinitialize();
	}
	exit(0);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
    int miiNum = -1,index = 0,i;
	mii Miis[MAX_MII_NUM];
	char saveDir[] = "sd:/MIIs";
	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0H");
	printf("+---------------------+\n");
	printf("|  Mii Dumper v1.0.3  |\n");
    printf("| developed by Kazuki |\n");
    printf("+---------------------+\n");
	miiNum = readMiis(Miis);
	if(miiNum <= 0)goto error;
	printf("[*] Mounting SD Card...");
    if (SD_Initialize()){
	    printf(" OK!\n");
		sdInitialize = 1;
	    }else{
            printf(" Error!\n");
			miiNum = -1;
			goto error;
	    }
	DIR*pdir = opendir(saveDir);
	if(pdir){
        closedir(pdir);
	}else{
		if(mkdir(saveDir,0777) != 0){
			printf("Error:cannot create %s directory\n",saveDir);
			miiNum = -1;
		}
	}
	error:
	if(miiNum > 0){
        printf("\x1b[6;0H");
        for(i = 0;i < 21;i++){
            printf("                                        \n");
	    }
	    printf("\x1b[6;0H");
		printf("          Mii name  Birth   Favorite  \n");
		printf("                     day      color   \n");
	    printf("----------------------------------------------------------------------\n");
		printf("\x1b[%d;0H",SHOW_MII_NUM + 9);
		printf("----------------------------------------------------------------------\n");
		printf("\x1b[9;0H");
		showMiiTable(index,miiNum,Miis);
		printf("\x1b[%d;0H",SHOW_MII_NUM + 10);
		printf(FOOTER_LINE_1);
		printf(FOOTER_LINE_2);
	}else{
        if(miiNum == 0){
            printf("Mii is empty\n");
		}
            printf("\nPress HOME to exit\n");
	}
	while(1) {//メインループ

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application via exit
		if ( pressed & WPAD_BUTTON_HOME )appExit();
		if(miiNum > 0){
            if ( pressed & WPAD_BUTTON_UP ){
				index--;
				if(index < 0)index += miiNum;
                updateMiiList(index,miiNum,Miis);
			}else if(pressed & WPAD_BUTTON_DOWN){
                index++;
				if(index >= miiNum)index -= miiNum;
                updateMiiList(index,miiNum,Miis);
			}else if(pressed & WPAD_BUTTON_LEFT){
				index -= SHOW_MII_NUM;
				if(index < 0)index += miiNum;
				updateMiiList(index,miiNum,Miis);
			}else if(pressed & WPAD_BUTTON_RIGHT){
				index += SHOW_MII_NUM;
				if(index >= miiNum)index -= miiNum;
				updateMiiList(index,miiNum,Miis);
			}else if(pressed & WPAD_BUTTON_A){
                printf("\x1b[%d;0H",SHOW_MII_NUM + 10);
	            for(i = 0;i < 2;i++){
                    printf("                                              \n");
	            }
	            printf("\x1b[%d;0H",SHOW_MII_NUM + 10);
				printf("Dumping Mii...");
				if(!miiFileWrite(Miis,index,saveDir)){
                    printf("Success!\n");
					printf("Mii was dumped to %s/%08d.MII\n",saveDir,index + 1);
				}
			}
		}
		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}
