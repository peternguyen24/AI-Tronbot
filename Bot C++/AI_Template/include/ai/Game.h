#pragma once

#include "easywsclient.hpp"

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include "defines.h"
#include "AI.h"

#if defined _DEBUG
#define LOG(fmt, ...)  printf(fmt, __VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif

using namespace std;
using easywsclient::WebSocket;

/// @class Game class
class Game
{
private:
	// +------------------+------+
	// |       1 byte     |1 byte|
	// +------------------+------+
	// | COMMAND_SEND_KEY |  Key |
	// +------------------+------+
	/// Register AI with server.
	/// @return No return value.
	void AI_Register();

	/// Callback function to handle message from server.
	/// @param msg message received from server.
	/// @return No return value.
	static void OnMessage(const std::string & msg);

public:
	/// Game instance.
	static Game* s_instance;

	/// Server address.
	static std::string host;

	/// Port to connect to server.
	static unsigned int port;

	/// Key to authen with server.
	static unsigned int key;

	/// Websocket client instance.
	static WebSocket::pointer wsClient;

	static void CreateInstance()
    {
        if ( s_instance == NULL )
			s_instance = new Game();
    }
	
	static Game* GetInstance()
	{
		return s_instance;
	}

	static void DestroyInstance()
	{
		if (s_instance)
	   {
		 delete s_instance;
		 s_instance = NULL;
	   }
	}

	Game();
	~Game();

	/// Create a websocket connection to server.
	/// @param argc number of arguments.
	/// @param argv pointer to array of arguments. Format: -k <host> -p <port> -k <key>
	/// @return -1 if failed to connect, otherwise return 0.
	int Connect(int argc, char* argv[]);

	/// Polling from server
	/// @return No return value.
	void PollingFromServer();

	// +------------------------+------------+
	// |          1 byte        |   1 byte   |
	// +------------------------+------------+
	// | COMMAND_SEND_DIRECTION |  Direction |
	// +------------------------+------------+
	/// Function to make a move.
	/// @param direction specify the direction you want to move your bot.\n
	/// Valid direction is one of these: DIRECTION_LEFT, DIRECTION_UP, DIRECTION_RIGHT or DIRECTION_DOWN.
	/// @return No return value.
	void AI_Move(int direction);
};