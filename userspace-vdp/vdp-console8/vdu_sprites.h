#ifndef _VDU_SPRITES_H_
#define _VDU_SPRITES_H_

#include <Arduino.h>
#include <fabgl.h>

#include "sprites.h"
#include "vdp_protocol.h"

void receiveBitmap(int cmd, uint16_t width, uint16_t height) {
	//
	// Allocate new heap data
	//
	void * dataptr = (void *)heap_caps_malloc(sizeof(uint32_t)*width*height, MALLOC_CAP_SPIRAM);

	if (dataptr != NULL) {                  
		if (cmd == 1) {
			//
			// Read data to the new databuffer
			//
			for (int n = 0; n < width*height; n++) ((uint32_t *)dataptr)[n] = readLong_b();  
			debug_log("vdu_sys_sprites: bitmap %d - data received - width %d, height %d\n\r", getCurrentBitmap(), width, height);
		}
		if (cmd == 2) {
			uint32_t color = readLong_b();
			//
			// Define single color
			//
			for (int n = 0; n < width*height; n++) ((uint32_t *)dataptr)[n] = color;            
			debug_log("vdu_sys_sprites: bitmap %d - set to solid color - width %d, height %d\n\r", getCurrentBitmap(), width, height);            
		}
		// Create bitmap structure
		//
		createBitmap(width,height,dataptr);
	}
	else { 
		//
		// Discard incoming serial data if failed to allocate memory
		//
		if (cmd == 1) {
			discardBytes(4*width*height);
		}
		if (cmd == 2) {
			discardBytes(4);
		}
		debug_log("vdu_sys_sprites: bitmap %d - data discarded, no memory available - width %d, height %d\n\r", getCurrentBitmap(), width, height);
	}

}

// Sprite Engine
//
void vdu_sys_sprites(void) {
	int cmd = readByte_t();

	switch(cmd) {
		case 0: {	// Select bitmap
			int	rb = readByte_t();
			if (rb >= 0) {
				setCurrentBitmap(rb);
				debug_log("vdu_sys_sprites: bitmap %d selected\n\r", getCurrentBitmap());
			}
		}	break;

		case 1:		// Send bitmap data
		case 2: {	// Define bitmap in single color
			int rw = readWord_t(); if (rw == -1) return;
			int rh = readWord_t(); if (rh == -1) return;

			receiveBitmap(cmd, rw, rh);

		}	break;

		case 3: {	// Draw bitmap to screen (x,y)
			int	rx = readWord_t(); if (rx == -1) return;
			int ry = readWord_t(); if (ry == -1) return;

			drawBitmap(rx,ry);
			debug_log("vdu_sys_sprites: bitmap %d draw command\n\r", getCurrentBitmap());
		}	break;

	   /*
		* Sprites
		* 
		* Sprite creation order:
		* 1) Create bitmap(s) for sprite, or re-use bitmaps already created
		* 2) Select the correct sprite ID (0-255). The GDU only accepts sequential sprite sets, starting from ID 0. All sprites must be adjacent to 0
		* 3) Clear out any frames from previous program definitions
		* 4) Add one or more frames to each sprite
		* 5) Activate sprite to the GDU
		* 6) Show sprites on screen / move them around as needed
		* 7) Refresh
		*/
		case 4: {	// Select sprite
			int b = readByte_t(); if (b == -1) return;
			setCurrentSprite(b);
			debug_log("vdu_sys_sprites: sprite %d selected\n\r", getCurrentSprite());
		}	break;

		case 5: {	// Clear frames
			clearSpriteFrames();
			debug_log("vdu_sys_sprites: sprite %d - all frames cleared\n\r", getCurrentSprite());
		}	break;

		case 6:	{	// Add frame to sprite
			int b = readByte_t(); if (b == -1) return;
			addSpriteFrame(b);
			debug_log("vdu_sys_sprites: sprite %d - bitmap %d added as frame %d\n\r", getCurrentSprite(), b, getSprite()->framesCount-1);
		}	break;

		case 7:	{	// Active sprites to GDU
			int b = readByte_t(); if (b == -1) return;
			activateSprites(b);
			debug_log("vdu_sys_sprites: %d sprites activated\n\r", numsprites);
		}	break;

		case 8: {	// Set next frame on sprite
			nextSpriteFrame();
			debug_log("vdu_sys_sprites: sprite %d next frame\n\r", getCurrentSprite());
		}	break;

		case 9:	{	// Set previous frame on sprite
			previousSpriteFrame();
			debug_log("vdu_sys_sprites: sprite %d previous frame\n\r", getCurrentSprite());
		}	break;

		case 10: {	// Set current frame id on sprite
			int b = readByte_t(); if (b == -1) return;
			setSpriteFrame(b);
			debug_log("vdu_sys_sprites: sprite %d set to frame %d\n\r", getCurrentSprite(), b);
		}	break;

		case 11: {	// Show sprite
			showSprite();
			debug_log("vdu_sys_sprites: sprite %d show cmd\n\r", getCurrentSprite());
		}	break;

		case 12: {	// Hide sprite
			hideSprite();
			debug_log("vdu_sys_sprites: sprite %d hide cmd\n\r", getCurrentSprite());
		}	break;

		case 13: {	// Move sprite to coordinate on screen
			int	rx = readWord_t(); if (rx == -1) return;
			int ry = readWord_t(); if (ry == -1) return;
			moveSprite(rx, ry);
			debug_log("vdu_sys_sprites: sprite %d - move to (%d,%d)\n\r", getCurrentSprite(), rx, ry);
		}	break;

		case 14: {	// Move sprite by offset to current coordinate on screen
			int	rx = readWord_t(); if (rx == -1) return;
			int ry = readWord_t(); if (ry == -1) return;
			moveSpriteBy(rx, ry);
			debug_log("vdu_sys_sprites: sprite %d - move by offset (%d,%d)\n\r", getCurrentSprite(), rx, ry);
		}	break;

		case 15: {	// Refresh
			refreshSprites();
			debug_log("vdu_sys_sprites: perform sprite refresh\n\r");
		}	break;

		case 16: {	// Reset
			cls(false);
			resetSprites();
			resetBitmaps();
			debug_log("vdu_sys_sprites: reset\n\r");
		}	break;
	}
}

#endif // _VDU_SPRITES_H_
