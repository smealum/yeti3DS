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

Handle srvHandle;

void aptInit()
{
	Handle aptuHandle;
	
	//initialize APT stuff, escape load screen
	srv_getServiceHandle(srvHandle, &aptuHandle, "APT:U");
	APT_GetLockHandle(aptuHandle, 0x0, NULL);
	svc_closeHandle(aptuHandle);
	svc_sleepThread(0x50000);
	
	srv_getServiceHandle(srvHandle, &aptuHandle, "APT:U");
	APT_Initialize(aptuHandle, 0x300, NULL, NULL);
	svc_closeHandle(aptuHandle);
	svc_sleepThread(0x50000);
	
	srv_getServiceHandle(srvHandle, &aptuHandle, "APT:U");
	APT_Enable(aptuHandle, 0x0);
	svc_closeHandle(aptuHandle);
	svc_sleepThread(0x50000);
}

u8* gspHeap;
u32* gxCmdBuf;
Handle gspGpuHandle;

u8 currentBuffer, leftOrRight;
u8* topFramebuffers[2][2];

void gspGpuGetFramebuffers()
{
	//grab main left screen framebuffer addresses
	GSPGPU_ReadHWRegs(gspGpuHandle, 0x400468, (u8*)&topFramebuffers[0], 8);
	GSPGPU_ReadHWRegs(gspGpuHandle, 0x400494, (u8*)&topFramebuffers[1], 8);

	//convert PA to VA (assuming FB in VRAM)
	topFramebuffers[0][0]+=0x7000000;
	topFramebuffers[0][1]+=0x7000000;
	topFramebuffers[1][0]+=0x7000000;
	topFramebuffers[1][1]+=0x7000000;
}

void gspGpuInit()
{
	//do stuff with GPU...
	srv_getServiceHandle(srvHandle, &gspGpuHandle, "gsp::Gpu");

	GSPGPU_AcquireRight(gspGpuHandle, 0x0);
	GSPGPU_SetLcdForceBlack(gspGpuHandle, 0x0);

	// //set framebuffer format
	u32 regData;
	// GSPGPU_ReadHWRegs(gspGpuHandle, 0x400470, (u8*)&regData, 4);
	// regData=(regData&0xFFFFFFF0)|3; //GL_RGB5_A1_OES
	// GSPGPU_WriteHWRegs(gspGpuHandle, 0x400470, (u8*)&regData, 4);
	// svc_sleepThread(0x50000);

	//set subscreen to blue
	regData=0x01FF0000;
	GSPGPU_WriteHWRegs(gspGpuHandle, 0x202A04, (u8*)&regData, 4);

	gspGpuGetFramebuffers();

	//setup our gsp shared mem section
	u8 threadID;
	Handle gspEvent, gspSharedMemHandle;
	svc_createEvent(&gspEvent, 0x0);
	GSPGPU_RegisterInterruptRelayQueue(gspGpuHandle, gspEvent, 0x1, &gspSharedMemHandle, &threadID);
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

void hidInit()
{
	Handle hidHandle, hidMemHandle;

	srv_getServiceHandle(srvHandle, &hidHandle, "hid:USER");

	HIDUSER_GetInfo(hidHandle, &hidMemHandle);
	svc_mapMemoryBlock(hidMemHandle, 0x10000000, 0x1, 0x10000000); //map HID shared mem to 0x10000000
	HIDUSER_Init(hidHandle);
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
	GSPGPU_ReadHWRegs(gspGpuHandle, 0x400478, (u8*)&regData, 4);
	regData^=1;
	currentBuffer=regData&1;
	GSPGPU_WriteHWRegs(gspGpuHandle, 0x400478, (u8*)&regData, 4);
}

void copyBuffer()
{
	//copy topleft FB
	u8 copiedBuffer=currentBuffer^1;
	u8* bufAdr=&gspHeap[0x46500*(currentBuffer+leftOrRight*2)];
	GSPGPU_FlushDataCache(gspGpuHandle, bufAdr, 0x46500);
	//GX RequestDma
	u32 gxCommand[0x8];
	gxCommand[0]=0x00; //CommandID
	gxCommand[1]=(u32)bufAdr; //source address
	gxCommand[2]=(u32)topFramebuffers[leftOrRight][copiedBuffer]; //destination address
	gxCommand[3]=0x46500; //size
	gxCommand[4]=gxCommand[5]=gxCommand[6]=gxCommand[7]=0x0;

	GSPGPU_submitGxCommand(gxCmdBuf, gxCommand, gspGpuHandle);
}

yeti_t yeti;

int main()
{
	getSrvHandle(&srvHandle);
	
	aptInit();

	gspGpuInit();
	hidInit();

	yeti_init(&yeti, (framebuffer_t*) &gspHeap[0x46500], (framebuffer_t*) gspHeap, textures, palette, lua);
	game_init(&yeti);

	while(1)
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

	svc_exitProcess();
	return 0;
}
