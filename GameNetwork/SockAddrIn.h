#pragma once

// SockAddrIn strcture
// It derives from sockaddr_storage in order to support IPv4 and IPv6
// Encapsulate SOCKADDR_STORAGE (IPv4: SOCKADDR_IN, IPv6: SOCKADDR_IN6) structures

// NOTES: IPv6 is not tested...

struct SockAddrIn : public sockaddr_storage
{
	SockAddrIn()
	{
		Clear();
	}

	SockAddrIn(const SockAddrIn& in)
	{
		Copy(in);
	}

	SockAddrIn(const sockaddr *addr, int len)
	{
		SetAddr(addr, len);
	}

	~SockAddrIn() {}

	SockAddrIn& Copy(const SockAddrIn& in)
	{
		ss_family = in.ss_family;
		memcpy(this, &in, Size());
		return *this;
	}

	void Clear()
	{
		memset(this, 0, Size());
	}

	bool IsEqual(const SockAddrIn& in) const
	{
		// Ignore sin_zero[8];
		if (ss_family == AF_INET)
		{
			return (memcmp(this, &in, Size() - 8) == 0);
		}

		return (memcmp(this, &in, Size()) == 0);
	}

	bool IsNull() const
	{
		return IsEqual(NullAddr);
	}

	ADDRESS_FAMILY GetFamily() const
	{
		return ss_family;
	}

	// Return the IPv4 network address;
	ULONG GetIPAddr() const
	{
		return ntohl(((SOCKADDR_IN *)this)->sin_addr.s_addr);
	}

	// Return port number;
	USHORT GetPort() const
	{
		return ntohs(((SOCKADDR_IN *)this)->sin_port);
	}

	// Get an IPv4 or IPv6 address from SockAddrIn structure;
	bool GetIPAddr(char *ipaddr, UINT nSize);

	// Create the SockAddrIn structure from host and service;
	// When host is NULL, it represents a local host;
	bool CreateFrom(const char *host, const char *service, int family = AF_INET);

	// Create IPv4 sockaddr from interger form of address and port number;
	bool CreateFrom(ULONG ipAddr, USHORT port, bool bFmtHost = true);

	// Get port number from service name;
	static USHORT GetPortNumber(const char *service);

	SockAddrIn& operator=(const SockAddrIn& in)
	{
		return Copy(in);
	}

	bool operator==(const SockAddrIn& in) const
	{
		return IsEqual(in);
	}

	bool operator!=(const SockAddrIn& in) const
	{
		return !IsEqual(in);
	}

	operator LPSOCKADDR()
	{
		return reinterpret_cast<LPSOCKADDR>(this);
  	}

	operator const IN6_ADDR*() const
	{
		return reinterpret_cast<const IN6_ADDR*>(this);
	}

	operator PIN6_ADDR()
	{
		return reinterpret_cast<PIN6_ADDR>(this);
	}

	size_t Size() const
	{
		return (ss_family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_storage);
	}

	// Initialize this object from SOCKADDR_IN;
	void SetAddr(const sockaddr_in *in)
	{
		SetAddr(reinterpret_cast<const sockaddr_storage *>(in));
	}

	// Initialize this object from SOCKADDR_IN6;
	void SetAddr(const sockaddr_in6 *in)
	{
		SetAddr(reinterpret_cast<const sockaddr_storage *>(in));
	}

	// Initialize this object from SOCKADDR_STORAGE;
	void SetAddr(const sockaddr_storage *in)
	{
		ss_family = in->ss_family;
		memcpy(this, in, Size());
	}

	void SetAddr(const sockaddr *addr, int len)
	{
		memcpy(this, addr, len);
	}

	// the hash function is used in hash<SockAddrIn> struct
	size_t GetHash() const
	{
		return (GetIPAddr()) |
			((static_cast<uint32_t>(GetPort())) << 13) |
			ss_family;
	}

	static SockAddrIn NullAddr;
};

typedef shared_ptr<SockAddrIn> SockAddrInPtr;

// the hash functor is used in NetServer class for unordered_map<SockAddrIn, ClientProxyPtr> AddrToClientMap,
// where SockAddrIn is used as hash key;
namespace std
{
	template<>
	struct hash<SockAddrIn>
	{
		size_t operator()(const SockAddrIn& addr) const
		{
			return addr.GetHash();
		}
	};
}