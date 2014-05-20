#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/APT.h>
#include <ctr/GSP.h>
#include <ctr/HID.h>
#include <ctr/svc.h>
#include "yeti.h"

u8* gspHeap;
u32* gxCmdBuf;

u8 currentBuffer, leftOrRight;
u8* topFramebuffers[2][2];

Handle gspEvent, gspSharedMemHandle;

void gspGpuGetFramebuffers()
{
	//grab main left screen framebuffer addresses
	GSPGPU_ReadHWRegs(NULL, 0x400468, (u8*)&topFramebuffers[0], 8);
	GSPGPU_ReadHWRegs(NULL, 0x400494, (u8*)&topFramebuffers[1], 8);

	//convert PA to VA (assuming FB in VRAM)
	topFramebuffers[0][0]+=0x7000000;
	topFramebuffers[0][1]+=0x7000000;
	topFramebuffers[1][0]+=0x7000000;
	topFramebuffers[1][1]+=0x7000000;
}

void gspGpuInit()
{
	gspInit();

	GSPGPU_AcquireRight(NULL, 0x0);
	GSPGPU_SetLcdForceBlack(NULL, 0x0);

	//set subscreen to blue
	u32 regData=0x01FF0000;
	GSPGPU_WriteHWRegs(NULL, 0x202A04, &regData, 4);

	gspGpuGetFramebuffers();

	//setup our gsp shared mem section
	u8 threadID;
	svc_createEvent(&gspEvent, 0x0);
	GSPGPU_RegisterInterruptRelayQueue(NULL, gspEvent, 0x1, &gspSharedMemHandle, &threadID);
	svc_mapMemoryBlock(gspSharedMemHandle, 0x10002000, 0x3, 0x10000000);

	//map GSP heap
	svc_controlMemory((u32*)&gspHeap, 0x0, 0x0, 0x2000000, 0x10003, 0x3);

	//wait until we can write stuff to it
	svc_waitSynchronization1(gspEvent, 0x55bcb0);

	//GSP shared mem : 0x2779F000
	gxCmdBuf=(u32*)(0x10002000+0x800+threadID*0x200);

	currentBuffer=0;
	leftOrRight=0;
}

void gspGpuExit()
{
	GSPGPU_UnregisterInterruptRelayQueue(NULL);

	//unmap GSP shared mem
	svc_unmapMemoryBlock(gspSharedMemHandle, 0x10002000);
	svc_closeHandle(gspSharedMemHandle);
	svc_closeHandle(gspEvent);
	
	gspExit();

	//free GSP heap
	svc_controlMemory((u32*)&gspHeap, (u32)gspHeap, 0x0, 0x2000000, MEMOP_FREE, 0x0);
}

void yetiUpdateKeyboard(yeti_t* y)
{
	u32 keys=((u32*)0x10000000)[7];

	y->keyboard.a       = keys&PAD_A;
	y->keyboard.b       = keys&PAD_B;
	y->keyboard.select  = keys&PAD_SELECT;
	y->keyboard.left    = keys&PAD_LEFT;
	y->keyboard.right   = keys&PAD_RIGHT;
	y->keyboard.up      = keys&PAD_UP;
	y->keyboard.down    = keys&PAD_DOWN;
	y->keyboard.r       = keys&PAD_R;
	y->keyboard.l       = keys&PAD_L;
}

void swapBuffers()
{
	u32 regData;
	GSPGPU_ReadHWRegs(NULL, 0x400478, &regData, 4);
	regData^=1;
	currentBuffer=regData&1;
	GSPGPU_WriteHWRegs(NULL, 0x400478, &regData, 4);
}

void copyBuffer()
{
	//copy topleft FB
	u8 copiedBuffer=currentBuffer^1;
	u8* bufAdr=&gspHeap[0x46500*(currentBuffer+leftOrRight*2)];
	GSPGPU_FlushDataCache(NULL, bufAdr, 0x46500);

	GX_RequestDma(gxCmdBuf, (u32*)bufAdr, (u32*)topFramebuffers[leftOrRight][copiedBuffer], 0x46500);
}

yeti_t yeti;

int main()
{
	initSrv();
	
	aptInit(APPID_APPLICATION);

	gspGpuInit();

	hidInit(NULL);

	aptSetupEventHandler();

	yeti_init(&yeti, (framebuffer_t*) &gspHeap[0x46500], (framebuffer_t*) gspHeap, textures, palette, lua);
	game_init(&yeti);

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		if(status == APP_RUNNING)
		{
			yeti.viewport.back = &gspHeap[0x46500*(currentBuffer+leftOrRight*2)];
			yeti.viewport.front = yeti.viewport.back;
			game_draw(&yeti);

			if(!leftOrRight){swapBuffers();game_tick(&yeti);yetiUpdateKeyboard(&yeti);gspGpuGetFramebuffers();}
			copyBuffer();
			if(topFramebuffers[0][0]!=topFramebuffers[1][0] && topFramebuffers[0][1]!=topFramebuffers[1][1])leftOrRight=!leftOrRight;
			else leftOrRight=0;
			// svc_sleepThread(1000000000);
		}
		else if(status == APP_SUSPENDING)
		{
			aptReturnToMenu();
		}
		else if(status == APP_SLEEPMODE)
		{
			aptWaitStatusEvent();
		}
		svc_sleepThread(16666666);
	}

	hidExit();
	gspGpuExit();
	aptExit();
	svc_exitProcess();
	return 0;
}
