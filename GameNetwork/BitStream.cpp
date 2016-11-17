#include "stdafx.h"
#include <algorithm>
#include "BitStream.h"

// Realloc buffer in bit;
void BitStream::ReallocBuffer(uint32_t bitCapacity)
{
	// allocate enough bytes for the required bitCapacity;
	uint32_t bytesToAlloc = (bitCapacity + 7) >> 3;

	if (m_Buffer == nullptr)
	{
		m_Buffer = static_cast<char *>(calloc(bytesToAlloc, 1));
	}
	else
	{
		char *tmp = m_Buffer;
		m_Buffer = static_cast<char *>(calloc(bytesToAlloc, 1));
		// in case the new bit capacity is less than the orignial one;
		memcpy(m_Buffer, tmp, min((m_BitCapacity + 7) >> 3, bytesToAlloc));
		free(tmp);
	}

	m_BitCapacity = bytesToAlloc << 3;
}

void OutputBitStream::WriteBits(uint8_t data, size_t bitCount)
{
	uint32_t nextBitHead = m_BitHead + static_cast<uint32_t>(bitCount);
	if (nextBitHead > m_BitCapacity)
	{
		ReallocBuffer(std::max(m_BitCapacity * 2, nextBitHead));
	}

	// calculate the byte offset into the buffer by dividing bit head by 8;
	// the bit offset is the last 3 bits;
	uint32_t byteOffset = m_BitHead >> 3;
	uint32_t bitOffset = m_BitHead & 0x7;

	// calculate which bits of the current byte to preserve;
	uint8_t currentMask = ~(0xff << bitOffset);

	// calculate how many bits are free for the new data in the current byte;
	uint8_t bitsFree = 8 - bitOffset;

	// the current byte is combined with the old data (last bitOffset bits in the byte) 
	// and the new data (the first bitfFree bits in the byte);
	m_Buffer[byteOffset] = (m_Buffer[byteOffset] & currentMask) | (data << bitOffset);

	// when there are more bits than the free bits to write,
	// write them in next byte;
	if (bitsFree < bitCount)
	{
		m_Buffer[byteOffset + 1] = data >> bitsFree;
	}

	m_BitHead = nextBitHead;
}

void OutputBitStream::WriteBits(const void* data, size_t bitCount)
{
	const char* srcByte = static_cast<const char *>(data);

	// write the bytes;
	while (bitCount > 8)
	{
		WriteBits(*srcByte, 8);
		srcByte++;
		bitCount -= 8;
	}

	// write the remaining bits;
	if (bitCount > 0)
	{
		WriteBits(*srcByte, bitCount);
	}
}

void InputBitStream::ReadBits(uint8_t& data, size_t bitCount)
{
	uint32_t nextBitHead = m_BitHead + static_cast<uint32_t>(bitCount);

	// calculate the byte offset into the buffer by dividing bit head by 8;
	// the bit offset is the last 3 bits;
	uint32_t byteOffset = m_BitHead >> 3;
	uint32_t bitOffset = m_BitHead & 0x7;

	// get the data bits in current byte and shift them to the right;
	data = static_cast<uint8_t>(m_Buffer[byteOffset]) >> bitOffset;

	uint8_t readBits = 8 - bitOffset;

	// if there are more bits in next byte, get them and put them in the left;
	if (readBits < bitCount)
	{
		data |= static_cast<uint8_t>(m_Buffer[byteOffset + 1]) << readBits;
	}

	// get the bits we really need;
	data &= ~(0xff << bitCount);

	m_BitHead = nextBitHead;
}

void InputBitStream::ReadBits(void* data, size_t bitCount)
{
	uint8_t* destByte = static_cast<uint8_t *>(data);

	// read the bytes;
	while (bitCount > 8)
	{
		ReadBits(*destByte, 8);
		++destByte;
		bitCount -= 8;
	}

	// read the remaining bits;
	if (bitCount > 0)
	{
		ReadBits(*destByte, bitCount);
	}
}
