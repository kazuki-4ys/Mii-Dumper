#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <dirent.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>
#include "mii.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

int SD_Initialize(){
	int ret = fatMountSimple("sd", &__io_wiisd);
	if(!ret) return ret;
	return 1;
}

void SD_Deinitialize(){
	fatUnmount("sd:/");
    __io_wiisd.shutdown();
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
    int miiNum,i,fail = 0;
	mii Miis[MAX_MII_NUM];
	char saveDir[] = "sd:/MIIs";
	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0H");
	printf("+---------------------+\n");
	printf("|   Mii Dumper v0.1   |\n");
    printf("| developed by Kazuki |\n");
    printf("+---------------------+\n");
	miiNum = readMiis(Miis);
	if(miiNum <= 0)goto error;
	printf("[*] Mounting SD Card...");
    if (SD_Initialize()){
	    printf(" OK!\n");
		//ディレクトリが存在しなかったら作成
		DIR*pdir = opendir(saveDir);
		if(pdir){
            closedir(pdir);
		}else{
			if(mkdir(saveDir,0777) != 0){
                printf("Error:mkdir\n");
				SD_Deinitialize();
				goto error;
			}
		}
		for(i = 0;i < miiNum;i++){
            if(miiFileWrite(Miis,i,saveDir) != 0){
                fail++;
				printf("Fail in dumping Mii (%d/%d)\n",i + 1,miiNum);
			}
		}
		printf("\n%d/%d Miis are dumped\n",miiNum - fail,miiNum);
		SD_Deinitialize();
	    }else{
            printf(" Error!\n");
	    }
	error:
	printf("\nPress HOME to exit\n");
	while(1) {//メインループ

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application via exit
		if ( pressed & WPAD_BUTTON_HOME )exit(0);

		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}
