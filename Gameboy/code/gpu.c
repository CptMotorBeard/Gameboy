#include "display.h"
#include "memory.h"
#include "opcodes.h"
#include "hardware.h"
#include "gpu.h"
#include "interrupts.h"
#include <stdio.h>

#pragma region DEBUG_VARIABLES
#define GPU_LOG_SIZE 67

int GPU_DEBUG = 0;
int WINDOW_LAYER_DEBUG = 0;
int BG_LAYER_DEBUG = 0;
int SPRITE_LAYER_DEBUG = 0;
int RECORDING_LOGS = 0;

char DEBUG_LOGS[40][GPU_LOG_SIZE];
int DEBUG_LOGS_CUR = 0;

WORD numbers_16[10][8] =
{
	// 0
		{
			0x0000,
			0x3FFC,
			0x300C,
			0x300C,
			0x300C,
			0x300C,
			0x300C,
			0x3FFC
		},
	// 1	
		{
			0x0000,
			0x000C,
			0x000C,
			0x000C,
			0x000C,
			0x000C,
			0x000C,
			0x000C
		},
	//2	
		{
			0x0000,
			0x3FFC,
			0x000C,
			0x000C,
			0x3FFC,
			0x3000,
			0x3000,
			0x3FFC
		},
	//3	
		{
			0x0000,
			0x3FFC,
			0x000C,
			0x000C,
			0x3FFC,
			0x000C,
			0x000C,
			0x3FFC
		},
	//4
		{
			0x0000,
			0x300C,
			0x300C,
			0x300C,
			0x3FFC,
			0x000C,
			0x000C,
			0x000C
		},
	//5
		{
			0x0000,
			0x3FFC,
			0x3000,
			0x3000,
			0x3FFC,
			0x000C,
			0x000C,
			0x3FFC
		},
	//6
		{
			0x0000,
			0x3FFC,
			0x3000,
			0x3000,
			0x3FFC,
			0x300C,
			0x300C,
			0x3FFC
		},
	//7
		{
			0x0000,
			0x3FFC,
			0x000C,
			0x000C,
			0x000C,
			0x000C,
			0x000C,
			0x000C
		},
	//8
		{
			0x0000,
			0x3FFC,
			0x300C,
			0x300C,
			0x3FFC,
			0x300C,
			0x300C,
			0x3FFC
		},
	//9	
		{
			0x0000,
			0x3FFC,
			0x300C,
			0x300C,
			0x3FFC,
			0x000C,
			0x000C,
			0x3FFC
		}
};

BYTE numbers_8[10][8] =
{
	//0
		{
			0x00,
			0xFF,
			0xC3,
			0xC3,
			0xC3,
			0xC3,
			0xC3,
			0xFF
		},
	//1
		{
			0x00,
			0x30,
			0x30,
			0x30,
			0x30,
			0x30,
			0x30,
			0x30
		},
	//2
		{
			0x00,
			0xFC,
			0x0C,
			0x0C,
			0xFC,
			0xC0,
			0xC0,
			0xFC
		},
	//3
		{
			0x00,
			0xFC,
			0x0C,
			0x0C,
			0xFC,
			0x0C,
			0x0C,
			0xFC
		},
	//4
		{
			0x00,
			0xC3,
			0xC3,
			0xC3,
			0xFF,
			0x03,
			0x03,
			0x03
		},
	//5
		{
			0x00,
			0xFF,
			0xC0,
			0xC0,
			0xFF,
			0x03,
			0x03,
			0xFF
		},
	//6
		{
			0x00,
			0xFF,
			0xC0,
			0xC0,
			0xFF,
			0xC3,
			0xC3,
			0xFF
		},
	//7
		{
			0x00,
			0xFF,
			0x03,
			0x03,
			0x03,
			0x03,
			0x03,
			0x03
		},
	//8
		{
			0x00,
			0xFF,
			0xC3,
			0xC3,
			0xFF,
			0xC3,
			0xC3,
			0xFF
		},
	//9
		{
			0x00,
			0xFF,
			0xC3,
			0xC3,
			0xFF,
			0x03,
			0x03,
			0xFF
		}
};

void DEBUG_GPU()
{
	GPU_DEBUG = !GPU_DEBUG;
}

void TOGGLE_WINDOW_LAYER()
{
	WINDOW_LAYER_DEBUG = !WINDOW_LAYER_DEBUG;
}

void TOGGLE_BG_LAYER()
{
	BG_LAYER_DEBUG = !BG_LAYER_DEBUG;
}

void TOGGLE_SPRITE_LAYER()
{
	SPRITE_LAYER_DEBUG = !SPRITE_LAYER_DEBUG;
}

void RECORD_GPU_LOGS()
{
	if (RECORDING_LOGS)
	{
		// write to file
		FILE *fp;
		fopen_s(&fp, "DEBUG_LOGS.txt", "w");

		fwrite(&DEBUG_LOGS, sizeof(char) * GPU_LOG_SIZE, 40, fp);

		DEBUG_LOGS_CUR = 0;
		int i;
		char empty[GPU_LOG_SIZE];
		for (i = 0; i < GPU_LOG_SIZE; i++)
		{
			empty[i] = ' ';
		}

		for (i = 0; i < 40; i++)
		{
			*DEBUG_LOGS[i] = *empty;
		}

		fclose(fp);
	}
	RECORDING_LOGS = !RECORDING_LOGS;
}
#pragma endregion

/*
0xFF40
  Each bit of 0xFF40 represents various parameters with a 0 or 1 value
  Bit 0 - BG & Window display OFF / ON
  Bit 1 - OBJ (Sprite) display OFF / ON
  Bit 2 - OBJ (Sprite) size 8x8 or 8x16 (width x height)
  Bit 3 - BG Tile Map Display Select 0x9800 - 0x9BFF or 0x9C00 - 0x9FFF
  Bit 4 - BG & Window Tile Data Select 0x8800 - 0x97FF or 0x8000 - 0x8FFF
  Bit 5 - Window Display OFF / ON
  Bit 6 - Window Tile Map Display Select 0x9800 - 0x9BFF or 0x9C00 - 0x9FFF
  Bit 7 - LCD Control Operation OFF / ON

0xFF42
0xFF43
  The scrollX and scrollY bytes are values between 0 and 255 which tell us where the LCD screen displays

0xFF44
  The GPU controls this value, LCDC_Y, to indicate the current Y position of the LCD
  If the CPU writes to this location it resets, and CPU is using hardware.writeMemory()
  to write to memory. We would use that function in other places, but we need complete
  control over the value so we directly modify it using cpu[LCDC_Y_BYTE]++
*/

#define LCDC_BYTE     0xFF40
#define SCROLL_Y_BYTE 0xFF42
#define SCROLL_X_BYTE 0xFF43
#define LCDC_Y_BYTE   0xFF44

/* GPU MODES */
//Send head to first row (204 cycles)
#define HBLANK 0
#define HBLANK_CYCLES 204
//Send head to first column (4560 cycles), 10 cycles at 456 each
#define VBLANK 1
#define VBLANK_CYCLES 456
// Any values of LCDC_Y between 144 and 153 indicates the V-Blank period
#define VBLANK_START 144
#define VBLANK_END   153
//This time is used to fetch data from the Object Attribute Memory (80 cycles)
#define OAMLOAD 2
#define OAMLOAD_CYCLES 80
//Takes loaded line and draws it on screen (172 cycles)
#define LCD 3
#define LCD_CYCLES 172

int mMode = OAMLOAD;
int mGpuClock = 0;

// 3 values for a pixel for each pixel for our screen width
GLfloat mCurrentLinePixels[SCREEN_WIDTH * 3];

// The gameboy handles four different colours. Black (Pixel OFF), White (Pixel ON),
// Dark Grey (33% ON) and Light Grey (66% ON).
float mColourPalette[4][3] =
{
	// R      G       B
	  {1.0f,  1.0f,   1.0f},
	  {0.66f, 0.66f,  0.66f},
	  {0.33f, 0.33f,  0.33f},
	  {0.0f,  0.0f,   0.0f} // - percent of 255 as handled by openGL
};

/*
Tile map is simply 32*32 bytes refering to a certain tile in the tileset (results to a 256*256 display)

VRAM MAP
-------------------------------
9C00-9FFF Tile Map #1
-------------------------------
9800-9BFF Tile Map #0
-------------------------------
9000-97FF Tile Set #1 (tiles 0-127)
-------------------------------
8800-8FFF Tile set #1 (tiles 128-255)
		  Tile set #0 (tiles (-1)-(-128)
-------------------------------
8000-87FF Tile Set #0 (0-127)
-------------------------------

LCDC keeps track of the Display operation, tile maps, tile sets, and sprite/tile size
*/

/*
	BYTE 0	- y-coordinate of top-left corner, value stored is y-coordinate - 16
	BYTE 1	- x-coordinate of top-left corner, value stored is x-coordinate - 8
	BYTE 2	- Data tile number
	BYTE 3	- Options
			Bit 7 - Sprite / Background priority (0: Above background, 1: Below Background)
			Bit 6 - Y-flip (0: Normal, 1: Vertical Flip)
			Bit 5 - X-flip (0: Normal, 1: Horizontal Flip)
			Bit 4 - Palette (0: OBJ Palette 0, 1: OBJ Palette 1)
*/
struct spriteOAM
{
	BYTE yCoord;
	BYTE xCoord;
	BYTE tileNumber;
	BYTE options;
};

void gpuStep()
{
	mGpuClock += clock;

	switch (mMode)
	{
	case OAMLOAD:
		if (mGpuClock >= OAMLOAD_CYCLES)
		{
			mGpuClock = 0;
			mMode = LCD;

			processLine();
		}
		break;

	case LCD:
		if (mGpuClock >= LCD_CYCLES)
		{
			mGpuClock = 0;
			mMode = HBLANK;

			// Bit 7 tells us if we need to render
			if (readMemory(LCDC_BYTE) & BIT_7)
			{
				renderScanline();
			}

			// Trigger an LCD interrupt after rendering the line
			if (interrupt.enable && INTERRUPTS_LCDSTAT)
			{
				interrupt.flags |= INTERRUPTS_LCDSTAT;
			}
		}
		break;

	case HBLANK:
		if (mGpuClock >= HBLANK_CYCLES)
		{
			mGpuClock = 0;

			cleanLine();
			cpu[LCDC_Y_BYTE]++;

			if (readMemory(LCDC_Y_BYTE) >= VBLANK_START)
			{
				// VBLANK
				mMode = VBLANK;

				// Trigger a VBLANK interrupt after rengering the image
				if (interrupt.enable && INTERRUPTS_VBLANK)
				{
					//drawScreen();
					interrupt.flags |= INTERRUPTS_VBLANK;
				}
			}
			else
			{
				// If we aren't at a VBLANK yet we restart the process
				mMode = OAMLOAD;
			}
		}
		break;

	case VBLANK:
		if (mGpuClock >= VBLANK_CYCLES)
		{
			mGpuClock = 0;
			cpu[LCDC_Y_BYTE]++;

			if (readMemory(LCDC_Y_BYTE) > VBLANK_END)
			{
				// Restart
				mMode = OAMLOAD;
				cpu[LCDC_Y_BYTE] = 0;
			}
		}
		break;
	}
}

void processBackgroundLayer()
{
	if (BG_LAYER_DEBUG)
	{
		return;
	}

	BYTE LCDC = readMemory(LCDC_BYTE);
	BYTE scrollY = readMemory(SCROLL_Y_BYTE);
	BYTE scrollX = readMemory(SCROLL_X_BYTE);
	BYTE currentLine = readMemory(LCDC_Y_BYTE);

	// Bit 0 tells us if we need to draw the background, if it's not enabled we can just leave
	if (!(LCDC & BIT_0))
	{
		return;
	}

	// Bit 3 of LCDC tells us the address range for the BG Tile Map
	// 0x9C00 - 0x9800 = 0x400, which means when the bit is active we just increase the range by 0x400
	WORD bgTileMapAddress = 0x9800 + ((LCDC >> 3 & 0x1) * 0x400);

	/*
	  There are 32 blocks with 8 bits each for both x and y direction.
	  Our screen size is 20 blocks wide and 18 blocks high, or 160x144
	  The base address (bgTileMapAddress) needs to be shifted based on scrollX and scrollY
	  This places us in the correct location for what we want to display.
	  If the screen is scrolled enough where our shift places some of the blocks within our screen >32 then the screen wraps around.

	  TODO: This is going to get really really funky to try and deal with wrap around. Find a way to deal with that. (Check out the second page linked)
	*/
	int yShift = ((scrollY + currentLine) / 8) * 32;
	int xShift = (scrollX / 8);
	bgTileMapAddress += yShift + xShift;

	/*
	  We need our current screen x and y position, handled by scrollX and scrollY
	  Our tiles are 8x8 pixels. scroll is a pixel location, so scroll % 8 gives us our tiles positions
	  The upper left of our screen (which is where we start) is set by 7 - scrollX
	*/
	SIGNED_BYTE currentXPosition = 7 - (scrollX % 8);
	BYTE currentYPosition = (scrollY + currentLine) % 8;

	BYTE pixelsWritten = 0;

	while (pixelsWritten < SCREEN_WIDTH)
	{
		// Get the address of our tile from the tileSet
		BYTE tileAddress = readMemory(bgTileMapAddress);

		BYTE upperTileBits;
		BYTE lowerTileBits;

		// Bit 4 of LCDC tells us where and how to find our tile pattern
		/*
		  Tile patterns are
		  taken from the Tile Data Table located either at
		  $8000-8FFF or $8800-97FF. In the first case, patterns
		  are numbered with unsigned numbers from 0 to 255 (i.e.
		  pattern #0 lies at address $8000). In the second case,
		  patterns have signed numbers from -128 to 127 (i.e.
		  pattern #0 lies at address $9000).
		*/

		int isFirstPattern = (LCDC >> 4 & 0x1);
		int address = 0;

		/*
		  The tileAddress we get from our bgTileMap are ordered from 0x00 - 0xFF. In memory, this address corresponds to a value 0x10 times bigger
		  0x8000 would be tileAddress 0x00, 0x8010 would be tile address 0x01 and so on. The reason for this is that there is 16 bytes for each tile (0x10)
		  and 2 bytes for each row of pixels, which gives us our 8x8 tiles.

		  The currentYPosition will tell us which row of pixels we are currently looking at, we multiply by 2 since there are 2 bytes per row.
		*/
		if (isFirstPattern)
		{
			address = 0x8000 + (tileAddress * 0x10) + (currentYPosition * 2);
		}
		else
		{
			address = 0x9000 + (((SIGNED_BYTE)tileAddress) * 0x10) + (currentYPosition * 2);
		}

		// We need a total of 2 bytes for each pixel row in our tiles
		upperTileBits = readMemory(address);
		lowerTileBits = readMemory(address + 1);

		// Each time this loops, a new tile is written. The entire screen is 20 8x8 tiles wide.
		while (currentXPosition >= 0)
		{
			/*
			  Tile data is stored in 16 bytes where every 2 bytes represents a line in the tile
			  The values inside these 2 bytes represents the colour of each pixel in the 8x8 tile
			  The data of 2 lines translates into one line of pixels of various colours
				upper bits: 010001  ->  030021
				lower bits: 010010
			  We receive a value between 0 and 3 for our pixel.
			*/

			BYTE pixel = ((upperTileBits >> currentXPosition) & 0x1) + ((lowerTileBits >> currentXPosition) & 0x1) * 2;

			/*
			  0xFF47 is our BG & Window palette data.
				Bit 7-6 - Data for Dot Data 11
				Bit 5-4 - Data for Dot Data 10
				Bit 3-2 - Data for Dot Data 01
				Bit 1-0 - Data for Dot Data 00
			*/

			BYTE palette = (readMemory(0xFF47) >> (pixel * 2)) & (BIT_0 | BIT_1);

			int currentPixelLocation = pixelsWritten * 3;
			// Each pixel location has 3 bits of data associated with it for rgb values
			mCurrentLinePixels[currentPixelLocation] = mColourPalette[palette][0];
			mCurrentLinePixels[currentPixelLocation + 1] = mColourPalette[palette][1];
			mCurrentLinePixels[currentPixelLocation + 2] = mColourPalette[palette][2];

			pixelsWritten++;
			currentXPosition--;
		}

		currentXPosition = 7;
		bgTileMapAddress++;
	}
}

void processWindowLayer()
{
	if (WINDOW_LAYER_DEBUG)
	{
		return;
	}

	BYTE LCDC = readMemory(LCDC_BYTE);
	BYTE winY = readMemory(0xFF4A);	// 0xFF4A is the window y coordinate; 0 <= WY <= 143
	BYTE winX = readMemory(0xFF4B);	// 0xFF4B is the window x coordinate; 7 <= WX <= 166. A value between 0-6 should not be allowed for WX
	winX -= 7;

	if (GPU_DEBUG)
	{
		winY = 0;
		winX = 0;
	}

	BYTE currentLine = readMemory(LCDC_Y_BYTE);

	// Our window is only drawn within the bounds of winY, winX
	if (currentLine < winY)
	{
		return;
	}

	// Bit 0 and bit 5 tells us if we need to draw the window layer, if they're not enabled we leave
	if (!((LCDC & BIT_0) || (LCDC & BIT_5)))
	{
		return;
	}

	// Bit 6 of LCDC tells us the address range for the Window Tile Map
	// 0x9C00 - 0x9800 = 0x400, which means when the bit is active we just increase the range by 0x400
	WORD windowTileMapAddress = 0x9800 + ((LCDC >> 6 & 0x1) * 0x400);

	/*
	  We need our current screen x and y position, handled by scrollX and scrollY
	  Our tiles are 8x8 pixels. scroll is a pixel location, so scroll % 8 gives us our tiles positions
	  The upper left of our screen (which is where we start) is set by 7 - scrollX
	*/
	SIGNED_BYTE currentXPosition = 7;
	BYTE currentYPosition = currentLine % 8;

	BYTE pixelsWritten = 0;

	while (pixelsWritten < SCREEN_WIDTH)
	{
		// TODO: This can probably be done better, but for now this is easiest

		// Do nothing while we pass through invalid draw zones
		if (pixelsWritten < winX)
		{
			pixelsWritten++;
			currentXPosition--;

			if (currentXPosition < 0)
			{
				currentXPosition = 7;
				windowTileMapAddress++;
			}
		}
		else
		{
			// Get the address of our tile from the tileSet
			BYTE tileAddress = readMemory(windowTileMapAddress);

			BYTE upperTileBits;
			BYTE lowerTileBits;

			// Bit 4 of LCDC tells us where and how to find our tile pattern
			/*
			  Tile patterns are
			  taken from the Tile Data Table located either at
			  $8000-8FFF or $8800-97FF. In the first case, patterns
			  are numbered with unsigned numbers from 0 to 255 (i.e.
			  pattern #0 lies at address $8000). In the second case,
			  patterns have signed numbers from -128 to 127 (i.e.
			  pattern #0 lies at address $9000).
			*/

			int isFirstPattern = (LCDC >> 4 & 0x1);
			int address = 0;

			/*
			  The tileAddress we get from our bgTileMap are ordered from 0x00 - 0xFF. In memory, this address corresponds to a value 0x10 times bigger
			  0x8000 would be tileAddress 0x00, 0x8010 would be tile address 0x01 and so on. The reason for this is that there is 16 bytes for each tile (0x10)
			  and 2 bytes for each row of pixels, which gives us our 8x8 tiles.

			  The currentYPosition will tell us which row of pixels we are currently looking at, we multiply by 2 since there are 2 bytes per row.
			*/
			if (isFirstPattern)
			{
				address = 0x8000 + (tileAddress * 0x10) + (currentYPosition * 2);
			}
			else
			{
				address = 0x9000 + (((SIGNED_BYTE)tileAddress) * 0x10) + (currentYPosition * 2);
			}

			// We need a total of 2 bytes for each pixel row in our tiles
			upperTileBits = readMemory(address);
			lowerTileBits = readMemory(address + 1);

			// Each time this loops, a new tile is written. The entire screen is 20 8x8 tiles wide.
			while (currentXPosition >= 0)
			{
				/*
				  Tile data is stored in 16 bytes where every 2 bytes represents a line in the tile
				  The values inside these 2 bytes represents the colour of each pixel in the 8x8 tile
				  The data of 2 lines translates into one line of pixels of various colours
					upper bits: 010001  ->  030021
					lower bits: 010010
				  We receive a value between 0 and 3 for our pixel.
				*/

				BYTE pixel = ((upperTileBits >> currentXPosition) & 0x1) + ((lowerTileBits >> currentXPosition) & 0x1) * 2;

				/*
				  0xFF47 is our BG & Window palette data.
					Bit 7-6 - Data for Dot Data 11
					Bit 5-4 - Data for Dot Data 10
					Bit 3-2 - Data for Dot Data 01
					Bit 1-0 - Data for Dot Data 00
				*/

				BYTE palette = (readMemory(0xFF47) >> (pixel * 2)) & (BIT_0 | BIT_1);

				int currentPixelLocation = pixelsWritten * 3;
				// Each pixel location has 3 bits of data associated with it for rgb values
				mCurrentLinePixels[currentPixelLocation] = mColourPalette[palette][0];
				mCurrentLinePixels[currentPixelLocation + 1] = mColourPalette[palette][1];
				mCurrentLinePixels[currentPixelLocation + 2] = mColourPalette[palette][2];

				pixelsWritten++;
				currentXPosition--;
			}

			currentXPosition = 7;
			windowTileMapAddress++;
		}
	}
}

void processSpriteLayer() {
	if (SPRITE_LAYER_DEBUG)
	{
		return;
	}

	BYTE LCDC = readMemory(LCDC_BYTE);

	// Bit 1 of LCDC tells us if the sprite display is on, return if not
	if (!(LCDC & BIT_1))
	{
		return;
	}

	// Width is always 8, but bit 2 will tell us if we need to increase the height by 8
	BYTE spriteXSize = 8;
	BYTE spriteYSize = 8 + ((LCDC >> 2 & 0x1) * 8);

	// Store our current line so we don't have to access the array each time
	BYTE currentYPosition = readMemory(LCDC_Y_BYTE);

	int i;
	struct spriteOAM currentSprite;

	// Cycle through each of the 40 possible sprites and display them if applicable
	for (i = 0; i < 40; i++)
	{
		int currentOAMSpriteNum = i * 4;
		currentSprite.yCoord = readMemory(0xFE00 + currentOAMSpriteNum);
		currentSprite.xCoord = readMemory(0xFE01 + currentOAMSpriteNum);
		currentSprite.tileNumber = readMemory(0xFE02 + currentOAMSpriteNum);
		currentSprite.options = readMemory(0xFE03 + currentOAMSpriteNum);

		int bgPriority = currentSprite.options >> 7 & 0x1;
		int yFlip = currentSprite.options >> 6 & 0x1;
		int xFlip = currentSprite.options >> 5 & 0x1;
		int objectPalette = currentSprite.options >> 4 & 0x1;

		// Check if the current sprite is on the line we are drawing.
		if ((currentSprite.yCoord - (16 - spriteYSize) > currentYPosition) && (currentSprite.yCoord - 16 <= currentYPosition))
		{
			//draw it on the line
			WORD tileAddr = 0x8000 + (currentSprite.tileNumber * 0x10);

			BYTE upperTileBits;
			BYTE lowerTileBits;

			BYTE curSpriteX = currentSprite.xCoord - 8;
			BYTE curSpriteY = currentSprite.yCoord - 16;

			int address;

			/*
				The tileAddress we get from our tile map are ordered from 0x00 - 0xFF. In memory, this address corresponds to a value 0x10 times bigger
				0x8000 would be tileAddress 0x00, 0x8010 would be tile address 0x01 and so on. The reason for this is that there is 16 bytes for each tile (0x10)
				and 2 bytes for each row of pixels, which gives us our 8x8 tiles.

				The currentYPosition will tell us which row of pixels we are currently looking at, we multiply by 2 since there are 2 bytes per row.
			*/
			int currentSpriteYPosition;
			currentSpriteYPosition = currentYPosition - curSpriteY;

			if (yFlip)
			{
				address = tileAddr + ((7 - currentSpriteYPosition) * 2);
			}
			else
			{
				address = tileAddr + (currentSpriteYPosition * 2);
			}

			upperTileBits = readMemory(address);
			lowerTileBits = readMemory(address + 1);

			int j;
			for (j = 0; j < 8; j++)
			{
				/*
					Tile data is stored in 16 bytes where every 2 bytes represents a line in the tile
					The values inside these 2 bytes represents the colour of each pixel in the 8x8 tile
					The data of 2 lines translates into one line of pixels of various colours
					upper bits: 010001  ->  030001
					lower bits: 010000
					We receive a value between 0 and 3 for our pixel.
				*/
				int pixel;
				if (xFlip)
				{
					pixel = ((upperTileBits >> j) & 0x1) + ((lowerTileBits >> j) & 0x1) * 2;
				}
				else
				{
					pixel = ((upperTileBits >> (7 - j)) & 0x1) + ((lowerTileBits >> (7 - j)) & 0x1) * 2;
				}

				// Replace all the sprites with their OAM number
				if (GPU_DEBUG)
				{
					int k = 0;

					k = currentYPosition - curSpriteY;

					if (i > 9)
					{
						int ones = i % 10;
						int tens = i / 10;

						if (j > 3)
						{
							pixel = ((numbers_8[ones][k]) >> ((3 - (j - 4)) * 2)) & 0x3;
						}
						else
						{
							pixel = ((numbers_8[tens][k]) >> ((3 - j) * 2)) & 0x3;
						}
					}
					else
					{
						pixel = ((numbers_16[i][k]) >> (7 - j) * 2) & 0x3;
					}
				}

				/*
					0xFF48 and 0xFF49 are our BG & Window palette data.
					Bit 7-6 - Data for Dot Data 11
					Bit 5-4 - Data for Dot Data 10
					Bit 3-2 - Data for Dot Data 01
					Bit 1-0 - Data for Dot Data 00
				*/

				BYTE palette = (readMemory(0xFF48 + objectPalette) >> (pixel * 2)) & (BIT_0 | BIT_1);

				// Each pixel location has 3 bits of data associated with it for rgb values
				int currentPixel = (curSpriteX + j) * 3;

				// Draw the sprite if it's within our screen size
				if ((curSpriteX >= 0) && (curSpriteX <= SCREEN_WIDTH) && (curSpriteY >= 0) && (curSpriteY < SCREEN_HEIGHT))
				{
					// We draw bgPriority pixels only if the background colour is white
					// TODO need to figure out how to get the current value of pixel for the background. If this value is 0 we draw the sprite.			
					// Look into how to retrieve information from the background map (cpu[0x9800/0x9C00])
					int bgPixelDrawn = bgPriority;
					// We draw fgPixels so long as they are not transparent
					// TODO for some reason sprites 10, 11, 12, 13, 14 are all using a transparent pixel of 1 instead of 0 on the level select of Dr Mario
					int fgPixelDrawn = !bgPriority && pixel != 0;

					if (bgPixelDrawn || fgPixelDrawn)
					{
						mCurrentLinePixels[currentPixel] = mColourPalette[palette][0];
						mCurrentLinePixels[currentPixel + 1] = mColourPalette[palette][1];
						mCurrentLinePixels[currentPixel + 2] = mColourPalette[palette][2];
					}
				}
			}
		}
		if (RECORDING_LOGS)
		{
			BYTE winY = readMemory(0xFF4A);
			BYTE winX = readMemory(0xFF4B);

			sprintf_s(DEBUG_LOGS[DEBUG_LOGS_CUR], GPU_LOG_SIZE, "Drawing sprite %2d at position (%3d , %3d). BGPrio %d. ObjPalette %d\n", i, winX, winY, bgPriority, objectPalette);
			DEBUG_LOGS_CUR++;
			if (DEBUG_LOGS_CUR >= 40)
			{
				DEBUG_LOGS_CUR = 0;
			}
		}
	}
	if (RECORDING_LOGS)
	{
		RECORD_GPU_LOGS();
	}
}

void processLine()
{
	processBackgroundLayer();
	processWindowLayer();
	processSpriteLayer();
}

/*Access memory, go pixel by pixel to produce the correct line
  sprite collision priority: smaller x coor > table order
  sprite cap per line -> 10
  sprites with x = 0, 168 or y=0, 160 are hidden
  as a result processing left to right is the correct order*/

  // Clean line will reset all pixels in a line back to white
void cleanLine()
{
	BYTE currentLine = readMemory(LCDC_Y_BYTE);
	int i;

	for (i = 0; i < SCREEN_WIDTH * 3; i++)
	{
		BYTE currentPixelLocation = (SCREEN_WIDTH * 3 * currentLine) + i;
		mCurrentLinePixels[currentPixelLocation] = 1.0f;
		mCurrentLinePixels[currentPixelLocation + 1] = 1.0f;
		mCurrentLinePixels[currentPixelLocation + 2] = 1.0f;
	}
}

void renderScanline()
{
	BYTE currentLine = readMemory(LCDC_Y_BYTE);
	scanLine(mCurrentLinePixels, 143 - currentLine);
}

typedef struct
{
	unsigned char signature[2];
	unsigned int filesize;
	short reserves[2];
	unsigned int bfoffset;
} fileheader;

typedef struct
{
	unsigned int biSize;
	unsigned int biWidth;
	unsigned int biHeight;
	short biPlanes;
	short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	unsigned int biXPelsPerMeter;
	unsigned int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
} bitmapheader;

typedef struct
{
	char b;
	char g;
	char r;
}pixel;

void makeBitmap(char filename[], unsigned int width, unsigned int height, int data[])
{
	unsigned int padSize = width * 3 % 4;
	unsigned int lineSize = width * 3 + padSize;

	// Create new file
	FILE *fp;
	fopen_s(&fp, filename, "w");

	// Setup the file header
	fileheader fh;
	fh.signature[0] = 'B';
	fh.signature[1] = 'M';
	fh.filesize = 54 + (lineSize)*height;
	fh.reserves[0] = 0;
	fh.reserves[1] = 0;
	fh.bfoffset = 0x36;

	// Write out the file header
	fwrite(&fh.signature, 1, 2 * sizeof(char), fp);
	fwrite(&fh.filesize, 1, sizeof(unsigned int), fp);
	fwrite(&fh.reserves, 1, 2 * sizeof(short), fp);
	fwrite(&fh.bfoffset, 1, sizeof(unsigned int), fp);

	// Set out the bitmap header
	bitmapheader bmh;
	bmh.biSize = 0x28;
	bmh.biWidth = width;
	bmh.biHeight = height;
	bmh.biPlanes = 0x01;
	bmh.biBitCount = 0x18;
	bmh.biCompression = 0;
	bmh.biSizeImage = 0x10;
	bmh.biXPelsPerMeter = 0x0b13;
	bmh.biYPelsPerMeter = 0x0b13;
	bmh.biClrUsed = 0;
	bmh.biClrImportant = 0;
	// Write out the bitmap header
	fwrite(&bmh.biSize, 1, sizeof(unsigned int), fp);
	fwrite(&bmh.biWidth, 1, sizeof(unsigned int), fp);
	fwrite(&bmh.biHeight, 1, sizeof(unsigned int), fp);
	fwrite(&bmh.biPlanes, 1, sizeof(short), fp);
	fwrite(&bmh.biBitCount, 1, sizeof(short), fp);
	fwrite(&bmh.biCompression, 1, sizeof(unsigned int), fp);
	fwrite(&bmh.biSizeImage, 1, sizeof(unsigned int), fp);
	fwrite(&bmh.biXPelsPerMeter, 1, sizeof(unsigned int), fp);
	fwrite(&bmh.biYPelsPerMeter, 1, sizeof(unsigned int), fp);
	fwrite(&bmh.biClrUsed, 1, sizeof(unsigned int), fp);
	fwrite(&bmh.biClrImportant, 1, sizeof(unsigned int), fp);
	// Content of image
	char *padding = malloc(padSize * sizeof(char));
	memset(&padding, 0, padSize);
	pixel current;
	unsigned int i;
	unsigned int j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			current.r = data[(i*width + j) * 3];
			current.g = data[(i*width + j) * 3 + 1];
			current.b = data[(i*width + j) * 3 + 2];
			fwrite(&current, 1, sizeof(pixel), fp);
		}
		fwrite(&padding, 1, padSize, fp);
	}
	// Close and return
	free(padding);
	fclose(fp);
}

void ExportScreen(char foldername[])
{
	int data[256 * 256 * 3];
	BYTE scrollX = readMemory(0xFF43);
	BYTE scrollY = readMemory(0xFF42);
	BYTE LCDC = readMemory(0xFF40);
	WORD bgTileMapAddress = 0x9800 + (((LCDC >> 3) & 1) * 0x400);
	BYTE tileAddr;
	WORD tile;

	int i;
	int j;
	int k;
	int l;
	int pixel;
	int offset;
	//apply the background map to the data array
	for (i = 0; i < 32; i++) {
		for (j = 0; j < 32; j++) {

			tileAddr = readMemory(bgTileMapAddress);
			//get this tile
			if ((LCDC >> 4) & 0x1) {
				tile = (readMemory(0x8000 + (tileAddr * 0x10)) << 8) + readMemory(0x8000 + (tileAddr * 0x10) + 1);
			}
			else {
				tile = (readMemory(0x9000 + (((SIGNED_BYTE)tileAddr) * 0x10)) << 8) + readMemory(0x9000 + (((SIGNED_BYTE)tileAddr) * 0x10) + 1);
			}
			//apply the tile to data row by row
			for (k = 0; k < 8; k++) {
				for (l = 0; l < 8; l++) {
					pixel = (tile >> (l) & 1) * 2 + (((tile >> (8 + l)) & 1));
					offset = ((255 - (k + i * 8)) * 256 + j * 8 + (7 - l)) * 3;
					data[offset] = (int)(mColourPalette[pixel][0] * 255);
					data[offset + 1] = (int)(mColourPalette[pixel][1] * 255);
					data[offset + 2] = (int)(mColourPalette[pixel][2] * 255);
				}
				if ((LCDC >> 4) & 0x1) {
					tile = (readMemory(0x8000 + (tileAddr * 0x10) + (k * 2)) << 8) + readMemory(0x8000 + (tileAddr * 0x10) + (k * 2) + 1);
				}
				else {
					tile = (readMemory(0x9000 + (((SIGNED_BYTE)tileAddr) * 0x10) + (k * 2)) << 8) + readMemory(0x9000 + (((SIGNED_BYTE)tileAddr) * 0x10) + (k * 2) + 1);
				}

			}
			bgTileMapAddress++;
		}
	}
	char filename[50];
	sprintf_s(filename, 50, "%s/BG.bmp", foldername);
	makeBitmap(filename, 256, 256, data);
}

//Read OAM and export all tiles
void fillOAMFolder(char foldername[])
{
	int i;
	int data[8 * 8 * 3];
	WORD tile;
	struct spriteOAM currentSprite;
	int k;
	int l;
	int offset;
	int pixel;
	for (i = 0; i < 40; i++)
	{
		//find tile number
		currentSprite.tileNumber = readMemory(0xFE02 + i * 4);

		//send tile data to data
		//get this tile
		//apply the tile to data row by row
		for (k = 0; k < 8; k++)
		{
			for (l = 0; l < 8; l++)
			{
				tile = (cpu[0x8000 + (currentSprite.tileNumber * 0x10) + (k * 2)] << 8) + cpu[0x8000 + (currentSprite.tileNumber * 0x10) + (k * 2) + 1];
				pixel = (tile >> (l) & 1) * 2 + (((tile >> (8 + l)) & 1));

				offset = (k * 8 + l) * 3;
				data[offset] = (int)(mColourPalette[pixel][0] * 255);
				data[offset + 1] = (int)(mColourPalette[pixel][1] * 255);
				data[offset + 2] = (int)(mColourPalette[pixel][2] * 255);
			}
		}
		char filename[50];
		sprintf_s(filename, 50, "%s/sprite %d.bmp", foldername, i);
		makeBitmap(filename, 8, 8, data);
	}
}