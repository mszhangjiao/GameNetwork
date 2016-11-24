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

void NetManager::ShutdownAndClose()
{
	if (m_UDPSockPtr && m_UDPSockPtr->IsOpen())
		m_UDPSockPtr->ShutdownAndClose();
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

// todo: how to read multiple packets per frame?
void NetManager::ReadIncomingPackets()
{
	char buffer[cPacketBufferSize];
	InputBitStream is(buffer, cPacketBufferSize * 8);
	SockAddrIn sockAddr;

	int receivedPackets = 0;
	//int totalReadBytes = 0;

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

		// simulate packet loss
		float randf = MathUtil::GetRandomFloat();

		if (randf >= m_DropPacketChance)
		{
			// simulate latency
			float simulatedReceivedTime = TimeUtil::Instance().GetTimef() + m_SimulatedLatency * MathUtil::GetRandomFloat();
			m_ReceivedPackets.emplace(is, sockAddr, simulatedReceivedTime);
		}
		else
		{
			ShowDroppedPacket(is, sockAddr);
			++m_DroppedNum;
		}

		++m_ReceivedNum;

		VERBO("Whether to drop[%d], drop rate[%6.2f%s], drop chance[%6.2f], total[%d], dropped[%d]",
			(randf < m_DropPacketChance), static_cast<float>(m_DroppedNum) / m_ReceivedNum * 100.f, "%",
			m_DropPacketChance, m_ReceivedNum, m_DroppedNum
			);
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
