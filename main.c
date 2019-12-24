#define ANIM_IDLE 0
#define ANIM_WAIT 1
#define ANIM_WALK 2
#define ANIM_RUN 3
#define ANIM_SKID 4
#define ANIM_LOOK 5
#define ANIM_DUCK 6
#define ANIM_BALL 7

#include <genesis.h>
#include <resources.h>

bool gameOn = FALSE;

int score = 0;
char label_score[6] = "SCORE\0";
char strPosX[16] = "0";
char str_x1[16] = "0";
char str_x2[16] = "0";
char strPosY[16] = "0";
char str_y1[16] = "0";
char str_y2[16] = "0";

//Screen
const int LEFT_EDGE = 0;
const int RIGHT_EDGE = 280;
const int TOP_EDGE = 0;
const int BOTTOM_EDGE = 224;

//Player
Sprite *player;
fix32 playerPosX = FIX32(32);
fix32 playerVelX = FIX32(0);
fix16 playerPosY = FIX16(128);
fix16 playerVelY = FIX16(0);
int playerWidth = 32;
int playerHeight = 38;
bool jumping = FALSE;

//Scrolling Map
const int MAP_WIDTH_TILES = 640;
int xPos = 0;
bool forward = 1;
Map *mapScrollA;
u16 scroll = 0;
u16 currentFrame = 0;
int direction = 0;

//Ground Collision
int groundColX[8] = {000, 408, 438, 504, 534, 695, 760, 959};
int groundColY[8] = {168, 168, 183, 183, 183, 168, 224, 168};
u16 groundColPoints = 8;

void updateDebug()
{
	
	int angleX = groundColX[2] - groundColX[1];
	int angleY = groundColY[2] - groundColY[1];
	fix16 slope = fix16Div(intToFix16(angleY), intToFix16(angleX));
	sprintf(strPosY, "%d", FIX16(1));
	sprintf(str_y1, "%d", angleY);
	sprintf(str_y2, "%d", angleX);
	
	VDP_clearTextBG(PLAN_B, 16, 6, 4);
	VDP_clearTextBG(PLAN_B, 16, 8, 4);
	VDP_clearTextBG(PLAN_B, 16, 10, 4);

	VDP_drawTextBG(PLAN_B, "yPos:", 0, 6);
	VDP_drawTextBG(PLAN_B, strPosY, 16, 6);
	VDP_drawTextBG(PLAN_B, "Y1:", 0, 8);
	VDP_drawTextBG(PLAN_B, str_y1, 16, 8);
	VDP_drawTextBG(PLAN_B, "Y2:", 0, 10);
	VDP_drawTextBG(PLAN_B, str_y2, 16, 10);
}
void init();
void scrollTest();
void setupPlayer();
void startGame();
void playerSetup();
void playerJump();
void playerGravity();
int testGroundCollision(int pos);
void playerMovement();
void frameCount(u16 frame);
static void myJoyHandler(u16 joy, u16 changed, u16 state);

int main()
{
	init();
	while (1)
	{
		frameCount(1);
		scrollTest();
		playerMovement();
		testGroundCollision(xPos);
		playerGravity();
		SPR_update();
		VDP_waitVSync();
		updateDebug();
		checkSlope();
	}
	return(0);
	
}

void init()
{
	//Input
	JOY_init();
	JOY_setEventHandler(&myJoyHandler);

	//Set up the map tilesets
	VDP_setScreenWidth320();
	VDP_setPlanSize(64, 32);
	VDP_setPalette(PAL1, mapTest.palette->data);
	VDP_loadTileSet(mapTest.tileset, 1, DMA);
	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	mapScrollA = unpackMap(mapTest.map, NULL);

	VDP_setMapEx(PLAN_A, mapScrollA, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, 1), 0, 0, 0, 0, 63, 28);
	//Set the background color
	//Manually sets a pallete colour to a hex code
	//First pallet is the background color
	//SHANE THIS IS USEFUL
	VDP_setPaletteColor(0, RGB24_TO_VDPCOLOR(0x6dc2ca));

	SPR_init(0, 0, 0);
	playerSetup();
}

void startGame()
{
	if (gameOn == FALSE)
	{
		gameOn = TRUE;
	}
}

void playerSetup()
{
	VDP_setPalette(PAL2, sonic.palette->data);
	player = SPR_addSprite(&sonic, fix32ToInt(playerPosX), fix16ToInt(playerPosY), TILE_ATTR(PAL2, 0, FALSE, FALSE));
	SPR_update();
}

void playerJump()
{
	jumping = TRUE;
	playerVelY = FIX16(-4);
}

void playerGravity()
{
	// //Apply Velocity, need to use fix16Add to add two "floats" together
	playerPosY = fix16Add(playerPosY, playerVelY);

	//Apply gravity
	if (jumping == TRUE)
	{
		
	}

	//Check if player is on floor
	if (fix16ToInt(playerPosY) + playerHeight >= testGroundCollision(0))
	{
		jumping = FALSE;
	 	playerVelY = FIX16(0);
		playerPosY = intToFix16(testGroundCollision(0) - playerHeight);
	 }
	 else
	 {
		 playerVelY = fix16Add(playerVelY, 6);
	 }
}

int testGroundCollision(int pos)
{
	for (int i = 0; i < groundColPoints; i++)
	{
		if (xPos + playerWidth + fix32ToInt(playerPosX) < groundColX[i])
		{
			return groundColY[i];
		}
	}
	return 0;
}

int checkSlope()
{
	for (int i = 0; i < groundColPoints; i++)
	{
		if (xPos + playerWidth + fix32ToInt(playerPosX) >= groundColX[i])
		{
			char debug1[18];
			sprintf(debug1, "you are at edge %d", i);
			VDP_drawTextBG(PLAN_B, debug1, 16, 2);
			if(i>1 && i < groundColPoints)
			{
				if(direction != 0)
				{
					if(calcSlope(i, i+direction) == 0)
					{
						VDP_drawTextBG(PLAN_B, "nope", 16, 4);
					}else
					{

						VDP_drawTextBG(PLAN_B, "yup ", 16, 4);
					}
				}
				
			}	
		}
	}
	return 0;
}

int calcSlope(int point1, int point2)
{
	int angleX = groundColX[point2] - groundColX[point1];
	int angleY = groundColY[point2] - groundColY[point1];
	fix16 slope = fix16Div(intToFix16(angleY), intToFix16(angleX));

	if(slope > 32)
	{
		return 0;
	}
	return 1;
}

void playerMovement()
{
	//Add the player's velocity to its position
	playerPosX = fix32Add(playerPosX,playerVelX);
	if (playerPosX < FIX32(LEFT_EDGE + (RIGHT_EDGE / 6)))
	{
	 	playerPosX = FIX32(LEFT_EDGE + (RIGHT_EDGE / 6));
		xPos -= 3;
	}

	if (playerPosX >= FIX32(RIGHT_EDGE/4))
	{
		playerPosX = FIX32((RIGHT_EDGE/4) - 1);
		xPos += 3;
	}

	//Keep the player within the bounds of the screen
	//if(playerPosX )
	//Tell the sprite engine to position the sprite
	if (fix32ToInt(playerVelX) > 0)
	{
		SPR_setHFlip(player, FALSE);
	}
	else if(fix32ToInt(playerVelX) < 0)
	{
		SPR_setHFlip(player, TRUE);
	}

	if (fix32ToInt(playerVelX) == 0 && jumping == FALSE)
	{
		SPR_setAnim(player, ANIM_IDLE);
		
	}
	else if(playerVelX !=0 && jumping == FALSE)
	{
		SPR_setAnim(player, ANIM_WALK);
	}

	if (fix16ToInt(playerVelY) !=0 && jumping == TRUE)
	{
		SPR_setAnim(player, ANIM_BALL);
	}

		SPR_setPosition(player, fix32ToInt(playerPosX), fix16ToInt(playerPosY));
}
void frameCount(u16 frame)
{
	currentFrame += frame;
	if(currentFrame > 60)
	{
		currentFrame = 0;
	}
}

static void myJoyHandler(u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			if (gameOn == FALSE)
			{
				startGame();
			}
		}
		//Set the player velocity if the left or right dpad are pressed;
		//Set velocity to 0 if no direction is pressed
		//State = This will be 1 if the button is currently pressed and 0 if it isn’t.
		if(state & BUTTON_C)
		{
			if(jumping == FALSE)
			{
				playerJump();
			}
		}

		if (state & BUTTON_RIGHT)
		{
			 playerVelX = FIX32(3);
			 direction = 1;
		}
		else if (state & BUTTON_LEFT)
		{
			playerVelX = FIX32(-3);
			direction = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if ((changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT))
			{
				playerVelX = FIX32(0);
				direction = 0;
			}
		}
	}
}

void scrollTest()
{
	 if (xPos >= MAP_WIDTH_TILES)
	 {
	 	xPos = MAP_WIDTH_TILES-1;
	 }
	if (xPos <= 0)
	{
		xPos = 1;
	}

	//Magic Scrolling Code!
	int scrolledTiles = xPos >> 3;
	int updateRow = scrolledTiles & 63;
	VDP_setHorizontalScroll(PLAN_A, -xPos);
	if (direction == 1)
	{

		 VDP_setMapEx(PLAN_A, mapScrollA, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, 1), 63 + updateRow, 0, scrolledTiles + 63, 0, 1, 28);
	}else if(direction == -1)
	{
		VDP_setMapEx(PLAN_A, mapScrollA, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, 1), updateRow -64, 0, scrolledTiles, 0, 1, 28);
	}
}