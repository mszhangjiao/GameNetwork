#pragma once

using namespace std;

class BitStream
{
public:
	virtual ~BitStream()
	{
		if (m_OwnBuffer && m_Buffer)
		{
			free(m_Buffer);
			m_Buffer = nullptr;
		}
	}

	const char* GetBuffer() const
	{
		return m_Buffer;
	}

	char* GetBuffer()
	{
		return m_Buffer;
	}

	uint32_t GetBitLength() const
	{
		return m_BitHead;
	}

	uint32_t GetByteLength() const
	{
		return (m_BitHead + 7) >> 3;
	}

	void ResetCapacity(uint32_t byteCapacity)
	{
		assert(m_BitCapacity >= byteCapacity << 3);

		m_BitCapacity = byteCapacity << 3;
		m_BitHead = 0;
	}

protected:
	BitStream()
		: m_Buffer(nullptr)
		, m_BitHead(0)
		, m_BitCapacity(0)
		, m_OwnBuffer(true)
	{
	}

	BitStream(const BitStream& bs)
		: m_Buffer(nullptr)
		, m_BitHead(bs.m_BitHead)
		, m_BitCapacity(bs.m_BitCapacity)
		, m_OwnBuffer(true)
	{
		ReallocBuffer(m_BitCapacity);
		memcpy(m_Buffer, bs.m_Buffer, GetCapacityInByte());
	}

	// construct BitStream with existing buffer;
	BitStream(char* buffer, uint32_t bitCount, bool ownBuffer)
		: m_Buffer(buffer)
		, m_BitHead(0)
		, m_BitCapacity(bitCount)
		, m_OwnBuffer(ownBuffer)
	{
	}

	uint32_t GetCapacityInByte() const
	{
		return (m_BitCapacity + 7) >> 3;
	}

	void ReallocBuffer(uint32_t bitCapacity);

	char *m_Buffer;
	uint32_t m_BitHead;
	uint32_t m_BitCapacity;
	bool m_OwnBuffer;
};

class OutputBitStream : public BitStream
{
public:
	OutputBitStream()
		: BitStream()
	{
		ReallocBuffer(256);
	}

	// write a given number of bits from the byte into the bit stream;
	void WriteBits(uint8_t data, size_t bitCount);

	void WriteBits(const void* data, size_t bitCount);

	void WriteBytes(const void* data, size_t byteCount)
	{
		WriteBits(data, byteCount << 3);
	}

	void Write(bool data)
	{
		WriteBits(&data, 1);
	}

	void Write(const string& data)
	{
		uint32_t elemCount = static_cast<uint32_t>(data.size());

		Write(elemCount);
		for (auto elem : data)
		{
			Write(elem);
		}
	}

	// generic write for primitive data types;
	template <typename Message>
	void Write(const Message& data, size_t bitCount = sizeof(Message) << 3)
	{
		static_assert(is_arithmetic<Message>::value || is_enum<Message>::value, "Generic write only supports primitive data types");

		WriteBits(&data, bitCount);
	}

	// write vector of primitive data types;
	template <typename Message>
	void Write(const vector<Message>& vData)
	{
		size_t num = vData.size();
		Write(num);
		for (const auto& elem : vData)
		{
			Write(elem);
		}
	}

	template <typename Message, typename ...Types>
	void Write(const Message& firstArg, const Types& ...args)
	{
		Write(firstArg);
		Write(args...);
	}

	void Append(const OutputBitStream& os)
	{
		uint32_t bitCapacity = m_BitHead + os.m_BitHead;

		if (bitCapacity > m_BitCapacity)
		{
			ReallocBuffer(bitCapacity);
		}

		WriteBits(os.m_Buffer, os.m_BitHead);
	}
};

class InputBitStream : public BitStream
{
public:
	// use the passed in memory, don't manage its own buffer
	InputBitStream(char* buffer, uint32_t bitCount)
		: BitStream(buffer, bitCount, false)
	{
	}

	//InputBitStream(const InputBitStream* is)
	//	: BitSteam(dynamic_cast<BitStream*>(is))
	//{}

	void ReadBits(uint8_t& data, size_t bitCount);
	void ReadBits(void* data, size_t bitCount);
	void ReadBytes(void* data, size_t byteCount)
	{
		ReadBits(data, byteCount << 3);
	}

	void Read(bool &data)
	{
		ReadBits(&data, 1);
	}

	void Read(string& data)
	{
		uint32_t elemCount;

		Read(elemCount);
		data.resize(elemCount);
		for (auto& elem : data)
		{
			Read(elem);
		}
	}

	// generic read for primitive data types;
	template <typename Message>
	void Read(Message& data, size_t bitCount = sizeof(Message) << 3)
	{
		static_assert(is_arithmetic<Message>::value || is_enum<Message>::value, "Generic read only supports primitive data types");

		ReadBits(&data, bitCount);
	}

	// read vector of primitive data types;
	template <typename Message>
	void Read(vector<Message>& vData)
	{
		size_t num;
		Read(num);
		vData.resize(num);
		for (Message& elem : vData)
		{
			Read(elem);
		}
	}

	// variadic read function
	template <typename Message, typename ...Types>
	void Read(const Message& firstArg, const Types& ...args)
	{
		Read(firstArg);
		Read(args...);
	}
};
