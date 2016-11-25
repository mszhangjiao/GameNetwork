========================================================================
    CONSOLE APPLICATION : MiniGame Project Overview
========================================================================

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
========================================================================

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

========================================================================
How to run the game:

Executalbe: MiniGame.exe

It targets WIN32, both debug and release configs are built and running;

When you run the exe without command arguments or with wrong arguments, it will show:

Usage: 
	To run a game server: MiniGame -s
	To run a game client: MiniGame -c serverIP playerName
	To run unit test: MiniGame -t

It's not really a "game" (unlike what I planned to do), you don't have to input 
anything afte it's started. But it shows the test results of the reliable layer.

========================================================================
Game output

The console output are shown and explained as below.

Note: If you run the game in a debugger, you will find more DEBUG level logs in the Output
window.

Game client:
After a successful connection process, it starts showing heartbeat sent from the server.
The heartbeats are sent every 2 seconds, but the game simulated a random latency (0-100ms)
on both sides, so you can see the fluctuations between two ajacent heartbeats.

  0.00: INFO: Client Nancy is ready, connecting to server localhost ...
  1.08: INFO: Welcomed by the server, playerId: 0
  1.24: INFO: Server heartbeat: [   0]
  3.18: INFO: Server heartbeat: [   1]
  5.23: INFO: Server heartbeat: [   2]
  7.14: INFO: Server heartbeat: [   3]
  9.17: INFO: Server heartbeat: [   4]
 11.23: INFO: Server heartbeat: [   5]
 13.14: INFO: Server heartbeat: [   6]

Game Server:
After the player is ready, the server starts sending heartbeats to client. The stats are from 
a test with following parameters:

server drop chance: 10%, 
client drop chance: 10%, 
server latency: random, 0 - 100ms
client latency: random, 0 - 100ms

The resent rate is stably around 20- 25% (resulting from the server and client drop chance) after 
a period of time (> 2mins):

   0.00: INFO: Server is ready, waiting for players to join...
 16.70: INFO: Received HELLO from Nancy
 16.78: INFO: Nancy is ready: 1
 16.78: INFO: Stats for [ Nancy]: heartbeat[   1], resent rate[  0.00%], dispatched[   1], acked[   0], resent[   0]
 19.78: INFO: Stats for [ Nancy]: heartbeat[   2], resent rate[  0.00%], dispatched[   2], acked[   2], resent[   0]

...

103.78: INFO: Stats for [ Nancy]: heartbeat[  44], resent rate[ 15.91%], dispatched[  44], acked[  44], resent[   7]
106.78: INFO: Stats for [ Nancy]: heartbeat[  46], resent rate[ 17.39%], dispatched[  46], acked[  45], resent[   8]
109.78: INFO: Stats for [ Nancy]: heartbeat[  47], resent rate[ 21.28%], dispatched[  47], acked[  47], resent[  10]

...

133.78: INFO: Stats for [ Nancy]: heartbeat[  59], resent rate[ 22.03%], dispatched[  59], acked[  59], resent[  13]
136.78: INFO: Stats for [ Nancy]: heartbeat[  61], resent rate[ 21.31%], dispatched[  61], acked[  60], resent[  13]
139.78: INFO: Stats for [ Nancy]: heartbeat[  62], resent rate[ 20.97%], dispatched[  62], acked[  62], resent[  13]

