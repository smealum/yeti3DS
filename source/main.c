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

	y->keyboard.a       = (keys&KEY_A)!=0;
	y->keyboard.b       = (keys&KEY_B)!=0;
	y->keyboard.select  = (keys&KEY_SELECT)!=0;
	y->keyboard.left    = (keys&KEY_DLEFT)!=0;
	y->keyboard.right   = (keys&KEY_DRIGHT)!=0;
	y->keyboard.up      = (keys&KEY_DUP)!=0;
	y->keyboard.down    = (keys&KEY_DDOWN)!=0;
	y->keyboard.r       = (keys&KEY_R)!=0;
	y->keyboard.l       = (keys&KEY_L)!=0;
}

yeti_t yeti;

int main()
{
	srvInit();	
	aptInit();
	hidInit(NULL);
	irrstInit(NULL);
	gfxInit();

	yeti_init(
		&yeti,
		(framebuffer_t*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL),
		(framebuffer_t*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL),
		textures, palette, lua
	);

	gfxSet3D(true);

	game_init(&yeti);

	while(aptMainLoop())
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

		gspWaitForEvent(GSPEVENT_VBlank0, true);
	}

	gfxExit();
	irrstExit();
	hidExit();
	aptExit();
	srvExit();
	return 0;
}
