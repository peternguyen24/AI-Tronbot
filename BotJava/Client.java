// ==================== HOW TO RUN THIS =====================
// To build: Run #build.bat. Output file: Client.jar
// To run, call: java -jar Client.jar -h [host] -p [port] -k [key]
//
// If no argument given, it'll be 127.0.0.1:3011
// key is a secret string that authenticate the bot identity
// it is not required when testing
// ===========================================================

//////////////////////////////////////////////////////////////////////////////////////
//                                                                                  //
//                                    GAME RULES                                    //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////
// - Game board is an array of MAP_SIZExMAP_SIZE blocks                             //
// - 2 players starts at 2 corners of the game board                                //
// - Each player will take turn to move                                             //
// - Player can only move left/right/up/down and stay inside the game board         //
// - The game is over when one of 2 players cannot make a valid move                //
// - In a competitive match:                                                        //
//   + A player will lose if they cannot connect to server within 10 seconds        //
//   + A player will lose if they don't make a valid move within 3 seconds          //
//////////////////////////////////////////////////////////////////////////////////////

import java.lang.System;
import java.lang.String;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URI;
import javax.websocket.ContainerProvider;
import javax.websocket.DeploymentException;
import javax.websocket.Session;
import javax.websocket.WebSocketContainer;
import java.util.ArrayList;
import java.util.Random;

public class Client {
	// Remember to change DEBUG_BUILD to false when you build release
	public static final boolean DEBUG_BUILD = true;
	
	public static final char GAMESTATE_WAIT_FOR_PLAYER = 0;
	public static final char GAMESTATE_COMMENCING = 1;
	public static final char GAMESTATE_END = 2;
	
	public static final char COMMAND_SEND_KEY = 1;
	public static final char COMMAND_SEND_INDEX = 2;
	public static final char COMMAND_SEND_DIRECTION = 3;
	public static final char COMMAND_SEND_STAGE = 4;

	public static final char PLAYER_1 = 1;
	public static final char PLAYER_2 = 2;
	public static final char OBSERVER = 3;

	public static final int BLOCK_OUT_OF_BOARD = -1;
	public static final int BLOCK_EMPTY = 0;
	public static final int BLOCK_PLAYER_1 = 1;
	public static final int BLOCK_PLAYER_1_TRAIL = 2;
	public static final int BLOCK_PLAYER_2 = 3;
	public static final int BLOCK_PLAYER_2_TRAIL = 4;
	public static final int BLOCK_OBSTACLE = 5;

	public static final char DIRECTION_LEFT = 1;
	public static final char DIRECTION_UP = 2;
	public static final char DIRECTION_RIGHT = 3;
	public static final char DIRECTION_DOWN = 4;

	public static final int MAP_SIZE = 11;
	
	public static int CONVERT_COORD(int x, int y) {
		return y * MAP_SIZE + x;
	}
	
	public static void LOG(String s) {
		if (DEBUG_BUILD) {
			System.out.print(s);
		}
	}
	
	private static Client instance = null;
	protected Client() {}
	
	public static Client getInstance() {
		if(instance == null) {
			instance = new Client();
		}
		return instance;
	}
	
	Session session;
	
	
	/////////////////////////////////////////////////////////////////////////
	//                            USER'S PART                              //
	/////////////////////////////////////////////////////////////////////////
	
	// This function is called automatically each turn.
	// If it's your turn, remember to call AI_Move() with a valid move before the time is run out.
	// Please check for provided APIs in Javadoc/index.html
	public void AI_Update() {
		if(AI.getInstance().isMyTurn()) {
			int[] board = AI.getInstance().getBoard();	// Access block at (x, y) by using board[CONVERT_COORD(x,y)]
			Position myPos = AI.getInstance().getMyPosition();
			Position enemyPos = AI.getInstance().getEnemyPosition();

			//Just a silly bot with random moves
			ArrayList<Character> freeMoves = new ArrayList<Character>();
			if(myPos.x > 0 && AI.getInstance().getBlock(new Position(myPos.x - 1, myPos.y)) == BLOCK_EMPTY) {
				freeMoves.add(DIRECTION_LEFT);
			}
			if(myPos.x < MAP_SIZE-1 && AI.getInstance().getBlock(new Position(myPos.x + 1, myPos.y)) == BLOCK_EMPTY) {
				freeMoves.add(DIRECTION_RIGHT);
			}
			if(myPos.y > 0 && AI.getInstance().getBlock(new Position(myPos.x, myPos.y - 1)) == BLOCK_EMPTY) {
				freeMoves.add(DIRECTION_UP);
			}
			if(myPos.y < MAP_SIZE-1 && AI.getInstance().getBlock(new Position(myPos.x, myPos.y + 1)) == BLOCK_EMPTY) {
				freeMoves.add(DIRECTION_DOWN);
			}

			int size = freeMoves.size();
			if(size > 0) {
				Random r = new Random();
				int direction = (int)freeMoves.get(r.nextInt(size));
				LOG("Move: " + direction + "\n");

				//Remember to call AI_Move() within allowed time
				Game.getInstance().AI_Move(direction);
			}
		}
		else {
			// Do something while waiting for your opponent
		}
	}
	
	
	
	
	/////////////////////////////////////////////////////////////////////////
	//                        DO NOT TOUCH MAIN                            //
	/////////////////////////////////////////////////////////////////////////
	
	public static void main(String []args) {
		if (Game.getInstance().Connect(args) == 0) {
			Game.getInstance().AI_Register();
		
			Game.getInstance().PollingFromServer();
		}
		else {
			LOG("Failed to connect to server!\n");
			System.exit(1);
		}
		System.exit(0);
	}
}