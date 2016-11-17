#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <queue>
#include <map>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cassert>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::vector;
using std::queue;
using std::map;
using std::unordered_map;
using std::iostream;

#include "SockAddrIn.h"
#include "UDPSocket.h"
#include "SockUtil.h"
#include "BitStream.h"
#include "TimeUtil.h"
#include "NetManager.h"
#include "Message.h"
#include "Connection.h"

#define HOSTNAME_SIZE	MAX_PATH
