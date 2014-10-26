#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>

#include "yeti.h"

u8 leftOrRight;

void yetiUpdateKeyboard(yeti_t* y)
{
	hidScanInput();
	u32 keys=hidKeysHeld();

	y->keyboard.a       = keys&KEY_A;
	y->keyboard.b       = keys&KEY_B;
	y->keyboard.select  = keys&KEY_SELECT;
	y->keyboard.left    = keys&KEY_DLEFT;
	y->keyboard.right   = keys&KEY_DRIGHT;
	y->keyboard.up      = keys&KEY_DUP;
	y->keyboard.down    = keys&KEY_DDOWN;
	y->keyboard.r       = keys&KEY_R;
	y->keyboard.l       = keys&KEY_L;
}

yeti_t yeti;

int main()
{
	srvInit();	
	aptInit();
	hidInit(NULL);
	irrstInit(NULL);
	gfxInit();

	aptSetupEventHandler();

	yeti_init(
		&yeti,
		(framebuffer_t*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL),
		(framebuffer_t*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL),
		textures, palette, lua
	);

	gfxSet3D(true);

	game_init(&yeti);

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		if(status == APP_RUNNING)
		{
			int i;
			for(i=0;i<2;i++)
			{
				yeti.viewport.front = yeti.viewport.back;
				yeti.viewport.back = (framebuffer_t*)gfxGetFramebuffer(GFX_TOP, leftOrRight?GFX_LEFT:GFX_RIGHT, NULL, NULL);
			
				game_draw(&yeti);

				leftOrRight^=1;
			}

			yetiUpdateKeyboard(&yeti);
			game_tick(&yeti);

			if(hidKeysDown()&KEY_START)break;

			gfxFlushBuffers();
			gfxSwapBuffers();
		}
		else if(status == APP_SUSPENDING)
		{
			aptReturnToMenu();
		}
		else if(status == APP_SLEEPMODE)
		{
			aptWaitStatusEvent();
		}
		gspWaitForEvent(GSPEVENT_VBlank0, false);
	}

	gfxExit();
	irrstExit();
	hidExit();
	aptExit();
	srvExit();
	return 0;
}
