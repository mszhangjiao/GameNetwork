========================================================================
    STATIC LIBRARY : GameNetwork Project Overview
========================================================================

GameNetwork is a library implementing basic reliable UDP network that can be
used for other projects, like multiplayer games. 

The main features:

- It's on top of UDP socket, support a common socket address for both IPv4
and IPv6 (note that IPv6 is not tested);
- It defines a network manager to send and receive packets;
- It provide a reliable layer that guarantees the packets are sent and in order;
- It implements a bit stream system where data are written into and read from
bit streams, that optimizes the bandwidth;
- It defines a messaging system using variadic templates, you can define a new
message of type with a line of code (simply a typedef), and the serialization
of this new message type is ready for you. This uses the new feature from C++11,
relieve us from the ugly macros or repeatitive classes;
- It simulates packet loss and latency and do reliability statistics;

========================================================================

File descriptions:

- SharedHeader.h
This file includes headers used in this project. the headers are in order from 
common to specific, the order also considers the dependency among them;

This file should be included in other stdafx.h files of this project and other 
dependent projects.

- SockAddrIn.h
SockAddrIn class derives from sockaddr_storage in order to support IPv4 and IPv6
Encapsulate SOCKADDR_STORAGE (IPv4: SOCKADDR_IN, IPv6: SOCKADDR_IN6) structures

NOTES: IPv6 is not tested...

- UDPSocket.h/.cpp
This is a simple class for UDP socket, currently it only contains the basic functions 
necessary for the demo project;

- NetManager.h/.cpp
NetManager is the base class for networking driver, it defines common networking 
functionalities: it manages a UDP socket, sends and receives packets;

- BitStream.h/.cpp
BitStream is the base class for OuputBitStream and InputBitStream, mainly manage 
the stream buffer;

OutputBitStream writes various types of data into bit stream, support variadic 
template write;

InputBitStream reads various types of data from bit stream, support variadic 
template read;

- Message.h
class Message implements Message using variadic template; each message type is defined 
with variadic template parameters. It uses OutputBitStream and InputBitStream for output 
and input; it calls Connection functions to write and read reliability data inside a packet;

Packet format:
MessageType: uint8_t
Reliable: bool [+ SequenceNumber]
HasAck: bool [+ AckRange]
Message data: variadic...

- Connection.h/.cpp
Connection class guanrantees the packets to be delivered and in order; the out-of-order 
packets will be discarded and acked, so the sender will resend the lost packets;

- Utility.h/c.pp
It defines SockUtil, LogUtil, TimeUtil and LogUtil