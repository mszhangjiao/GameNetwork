=======================================================================================
    CONSOLE APPLICATION : MiniGame Project Overview
=======================================================================================

MiniGame

- This project is based on the GameNetwork library;
- Implement a basic matchmaking system: after connecting to the server using
the functionalities in GameNetwork, the player can send a FindMatch message 
to initiate the matchmaking process. The server will try to find a match for
the player and fill the match with other players. When a match is full, the
server starts the match. And it moves from turn to turn till the end of the
match;
- It simulates a simple card game: the server starts a game with certain 
number of cards and players, it randomly shuffles the cards and sends the 
cards to each player; in each turn, each player plays a card, and the server
compares the card value and rewards scores accordingly. The scores are
accumulated on the server side till the end of the match, and server sends
the match results to the players;
- Implement a console command system to handle the game commands. The input
of the commands is in a seperate thread, and the execution of the commands 
is in the main  thread;
=======================================================================================

File descriptions:

MiniGame.cpp
Defines the game entry point, it will run game server or client according to the 
commandlines;

Game.h/.cpp
Common game functionalities, defines some virtul functions for the server and client
game to override.

GameServer.h/.cpp
Implement game server;

GameClient.h/.cpp
Implement game client;

NetServer.h/.cpp
Implements server network manager;

NetClient.h/.cpp
Implements client network manager;

=======================================================================================
How to run the game:

Executalbe: MiniGame.exe

It targets WIN32, both debug and release configs are built and running;

When you run the exe without command arguments or with wrong arguments, it will show:

Usage: 
	To run a game server: MiniGame -s CardNum PlayerNum
	To run a game client: MiniGame -c serverIP playerName
	To run unit test: MiniGame -t

This game is based on the reliable UDP and client-server network in GameNetwork library.

The server will start with certain card number and player limit and wait for clients to
connect it.

The client clients to the server, when the client number reaches the required player limit,
the server will start a match, and run the match turn by turn till the end of the match.

At the end the game, client can choose to replay a match or leave the game by inputting
the command when the game prompts.

=======================================================================================
Game output

When running server with commandline: MiniGame -s 12 2

Server output:

  0.00 INFO: Server is ready, waiting for players to join...
 14.19 INFO: Received HELLO from Jenny
 14.28 INFO: GameServer::FindMatch: Find Match[0] for player[Jenny]
 14.34 INFO: GameServer::MatchReady: player [ Jenny] is ready for match [0]
 23.30 INFO: Received HELLO from Nancy
 23.38 INFO: GameServer::FindMatch: Find Match[0] for player[Nancy]
 23.38 INFO: GameServer::FindMatch: Send player[1] about player [0]
 23.38 INFO: GameServer::FindMatch: Send player[0] about player [1]
 23.53 INFO: GameServer::MatchReady: player [ Nancy] is ready for match [0]
 23.53 INFO: GameServer::StartMatch: sending player[0] dealt cards
 23.53 INFO: GameServer::StartMatch: sending player[1] dealt cards
 23.53 INFO: Start turn: matchId[0], turnId[0]
 23.67 INFO: GameServer::PlayTurn: player[0] played card [1] in turn [0]
 23.67 INFO: GameServer::PlayTurn: validated player[0] card [1]
 23.70 INFO: GameServer::PlayTurn: player[1] played card [3] in turn [0]
 23.70 INFO: GameServer::PlayTurn: validated player[1] card [3]
 23.70 INFO: Turn [ 0] result: player [ Jenny], played card [ 1], turn score [ 0]
 23.70 INFO: Turn [ 0] result: player [ Nancy], played card [ 3], turn score [ 1]
 23.70 INFO: Start turn: matchId[0], turnId[1]
 23.84 INFO: GameServer::PlayTurn: player[1] played card [12] in turn [1]
 23.84 INFO: GameServer::PlayTurn: validated player[1] card [12]
 23.84 INFO: GameServer::PlayTurn: player[0] played card [2] in turn [1]
 23.84 INFO: GameServer::PlayTurn: validated player[0] card [2]
 23.84 INFO: Turn [ 1] result: player [ Jenny], played card [ 2], turn score [ 0]
 23.84 INFO: Turn [ 1] result: player [ Nancy], played card [12], turn score [ 1]
 23.84 INFO: Start turn: matchId[0], turnId[2]
 23.99 INFO: GameServer::PlayTurn: player[0] played card [4] in turn [2]
 23.99 INFO: GameServer::PlayTurn: validated player[0] card [4]
 23.99 INFO: GameServer::PlayTurn: player[1] played card [7] in turn [2]
 23.99 INFO: GameServer::PlayTurn: validated player[1] card [7]
 23.99 INFO: Turn [ 2] result: player [ Jenny], played card [ 4], turn score [ 0]
 23.99 INFO: Turn [ 2] result: player [ Nancy], played card [ 7], turn score [ 1]
 23.99 INFO: Start turn: matchId[0], turnId[3]
 24.54 INFO: GameServer::PlayTurn: player[1] played card [6] in turn [3]
 24.54 INFO: GameServer::PlayTurn: validated player[1] card [6]
 24.54 INFO: GameServer::PlayTurn: player[0] played card [8] in turn [3]
 24.54 INFO: GameServer::PlayTurn: validated player[0] card [8]
 24.54 INFO: Turn [ 3] result: player [ Jenny], played card [ 6], turn score [ 0]
 24.54 INFO: Turn [ 3] result: player [ Nancy], played card [ 8], turn score [ 1]
 24.54 INFO: Start turn: matchId[0], turnId[4]
 24.62 INFO: GameServer::PlayTurn: player[1] played card [5] in turn [4]
 24.62 INFO: GameServer::PlayTurn: validated player[1] card [5]
 24.66 INFO: GameServer::PlayTurn: player[0] played card [9] in turn [4]
 24.66 INFO: GameServer::PlayTurn: validated player[0] card [9]
 24.66 INFO: Turn [ 4] result: player [ Jenny], played card [ 5], turn score [ 0]
 24.66 INFO: Turn [ 4] result: player [ Nancy], played card [ 9], turn score [ 1]
 24.66 INFO: Start turn: matchId[0], turnId[5]
 24.75 INFO: GameServer::PlayTurn: player[0] played card [10] in turn [5]
 24.75 INFO: GameServer::PlayTurn: validated player[0] card [10]
 25.17 INFO: GameServer::PlayTurn: player[1] played card [11] in turn [5]
 25.17 INFO: GameServer::PlayTurn: validated player[1] card [11]
 25.17 INFO: Turn [ 5] result: player [ Jenny], played card [10], turn score [ 0]
 25.17 INFO: Match [ 0] result: player [ Jenny], total score [ 2]
 25.17 INFO: Turn [ 5] result: player [ Nancy], played card [11], turn score [ 1]
 25.17 INFO: Match [ 0] result: player [ Nancy], total score [ 4]
 25.17 INFO: GameServer::UpdateTurn: match [0] ended

When running 2 clients with commandline: 
	MiniGame -c localhost Jenny
and:
	MiniGame -c localhost Nancy

Client Jenny got output as below:

  0.01 INFO: Client Jenny is ready, connecting to server localhost ...
  1.02 INFO: Welcomed by the server, playerId: 0, request to find a match
  1.13 INFO: GameClient::HandleFindMatchReply: Find match[0], set player ready
 10.21 INFO: GameClient::HandleJoinMatch: player[1] joins match[0]
 10.45 INFO: GameClient::HandleStartMatch: get dealt cards from match[0]: 1 2 4 8 9 10
 10.45 INFO: GameClient::PlayTurn: turnId[0], current card list [ 1 2 4 8 9 10], played card[1]
 10.56 INFO: Turn [ 0] result: player [ Jenny], played card [ 1], turn score [ 0]
 10.56 INFO: Turn [ 0] result: player [ Nancy], played card [ 3], turn score [ 1]
 10.57 INFO: GameClient::PlayTurn: turnId[1], current card list [ 2 4 8 9 10], played card[2]
 10.70 INFO: Turn [ 1] result: player [ Jenny], played card [ 2], turn score [ 0]
 10.70 INFO: Turn [ 1] result: player [ Nancy], played card [12], turn score [ 1]
 10.74 INFO: GameClient::PlayTurn: turnId[2], current card list [ 4 8 9 10], played card[4]
 10.87 INFO: Turn [ 2] result: player [ Jenny], played card [ 4], turn score [ 0]
 10.87 INFO: Turn [ 2] result: player [ Nancy], played card [ 7], turn score [ 1]
 11.27 INFO: GameClient::PlayTurn: turnId[3], current card list [ 8 9 10], played card[8]
 11.45 INFO: Turn [ 3] result: player [ Nancy], played card [ 6], turn score [ 0]
 11.45 INFO: Turn [ 3] result: player [ Jenny], played card [ 8], turn score [ 1]
 11.45 INFO: GameClient::PlayTurn: turnId[4], current card list [ 9 10], played card[9]
 11.53 INFO: Turn [ 4] result: player [ Nancy], played card [ 5], turn score [ 0]
 11.53 INFO: Turn [ 4] result: player [ Jenny], played card [ 9], turn score [ 1]
 11.53 INFO: GameClient::PlayTurn: turnId[5], current card list [ 10], played card[10]
 12.05 INFO: Turn [ 5] result: player [ Jenny], played card [10], turn score [ 0]
 12.05 INFO: Turn [ 5] result: player [ Nancy], played card [11], turn score [ 1]
 12.08 INFO: Match [ 0] result: player [ Jenny], total score [ 2]
 12.08 INFO: Match [ 0] result: player [ Nancy], total score [ 4]
 12.08 PROMPT: What do you want next? "replay" or "leave"