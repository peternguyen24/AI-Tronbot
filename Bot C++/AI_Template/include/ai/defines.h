#pragma once
#define CONNECTING_TIME				10000
#define THINKING_TIME				3000
#define MAP_SIZE					11

/////////////////////////////
//       GAME STATES       //
/////////////////////////////
#define GAMESTATE_WAIT_FOR_PLAYER	0
#define GAMESTATE_COMMENCING		1
#define GAMESTATE_END				2

/////////////////////////////
//         COMMANDS        //
/////////////////////////////
#define COMMAND_SEND_KEY			1
#define COMMAND_SEND_INDEX			2
#define COMMAND_SEND_DIRECTION		3
#define COMMAND_SEND_STAGE			4

/////////////////////////////
//         PLAYERS         //
/////////////////////////////
#define PLAYER_1					1
#define PLAYER_2					2
#define OBSERVER					3

/////////////////////////////
//         BLOCKS          //
/////////////////////////////
#define BLOCK_OUT_OF_BOARD			-1
#define BLOCK_EMPTY					0
#define BLOCK_PLAYER_1				1
#define BLOCK_PLAYER_1_TRAIL		2
#define BLOCK_PLAYER_2				3
#define BLOCK_PLAYER_2_TRAIL		4
#define BLOCK_OBSTACLE				5

/////////////////////////////
//          MOVES          //
/////////////////////////////
#define DIRECTION_LEFT				1
#define DIRECTION_UP				2
#define DIRECTION_RIGHT				3
#define DIRECTION_DOWN				4

/// Convert coordinate (x,y) of a 2-dimension array into index in 1-dimension array.
/// @param x specify the horizontal offset, range in [0,MAP_SIZE-1]
/// @param y specify the vertical offset, range in [0,MAP_SIZE-1]
#define CONVERT_COORD(x,y)	((y) * MAP_SIZE + (x))