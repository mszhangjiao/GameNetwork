#include "stdafx.h"

NetManagerPtr NetManager::s_Instance = nullptr;

bool NetManager::Init()
{
	m_UDPSockPtr = SockUtil::CreateUDPSocket(m_Family);
	if (m_UDPSockPtr == nullptr)
	{
		return false;
	}

	int result = NO_ERROR;

	if (IsServer())
		result = m_UDPSockPtr->Bind(NULL, m_Service.c_str());

	if (result == NO_ERROR)
	{
		result = m_UDPSockPtr->SetNonBlockingMode(true);		
	}

	return result == NO_ERROR;
}

void NetManager::ProcessIncomingPackets()
{
	ReadIncomingPackets();
	ProcessQueuedPackets();
	UpdateBytesSentLastFrame();
}

void NetManager::SendPacket(const OutputBitStream& os, const SockAddrIn& addr)
{
	int sentByte = m_UDPSockPtr->SendTo(os.GetBuffer(), os.GetByteLength(), addr);
}

void NetManager::ReadIncomingPackets()
{
	char buffer[cPacketBufferSize];
	InputBitStream is(buffer, cPacketBufferSize * 8);
	SockAddrIn sockAddr;

	int receivedPackets = 0;
	int totalReadBytes = 0;

	while (receivedPackets++ < cMaxPacketsPerFrame)
	{
		int readBytes = m_UDPSockPtr->ReceiveFrom(buffer, cPacketBufferSize, sockAddr);

		if (readBytes == 0)
			break;

		if (readBytes < 0)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				SetLastError(error);
				HandleConnectionError(sockAddr);
			}
			break;
		}

		is.ResetCapacity(readBytes);
		m_ReceivedPackets.emplace(is, sockAddr, TimeUtil::Instance().GetTimef());
	}
}

void NetManager::ProcessQueuedPackets()
{
	while (!m_ReceivedPackets.empty())
	{
		ReceivedPacket& packet = m_ReceivedPackets.front();
		if (TimeUtil::Instance().GetTimef() > packet.m_ReceivedTime)
		{
			ProcessPacket(packet.m_InputBitStream, packet.m_FromAddress);
			m_ReceivedPackets.pop();
		}
		else
		{
			break;
		}
	}
}

void NetManager::UpdateBytesSentLastFrame()
{

}
