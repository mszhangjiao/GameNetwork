#pragma once

// This file includes headers used in this project
// the headers are in order from common to specific
// the order also considers the dependency among them

// this file should be included in other stdafx.h files of this project
// and other dependent projects

// C++ headers
#include <memory>
#include <string>
#include <iomanip>
#include <cstdint>
#include <vector>
#include <queue>
#include <map>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <random>
#include <iterator>

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::vector;
using std::queue;
using std::map;
using std::unordered_map;
using std::iostream;

// Windows headers
#ifdef WIN32
#define NOMINMAX

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#endif

// Project headers
#include "CriticalSection.h"
#include "SockAddrIn.h"
#include "UDPSocket.h"
#include "Utility.h"
#include "BitStream.h"
#include "NetManager.h"
#include "Connection.h"
#include "NetPlayer.h"
#include "Match.h"
#include "Message.h"
#include "Game.h"
#include "NetServer.h"
#include "NetClient.h"

#define HOSTNAME_SIZE	MAX_PATH
