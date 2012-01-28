#pragma once
#include "Config.h"
#include <stdexcept>

namespace AdressFamilly
{
	enum Enum
	{
		AppleTalk = 0x10,
		Atm = 0x16,
		Banyan = 0x15,
		Ccitt = 10,
		Chaos = 5,
		Cluster = 0x18,
		DataKit = 9,
		DataLink = 13,
		DecNet = 12,
		Ecma = 8,
		FireFox = 0x13,
		HyperChannel = 15,
		Ieee12844 = 0x19,
		ImpLink = 3,
		InterNetwork = 2,
		InterNetworkV6 = 0x17,
		Ipx = 6,
		Irda = 0x1a,
		Iso = 7,
		Lat = 14,
		Max = 0x1d,
		NetBios = 0x11,
		NetworkDesigners = 0x1c,
		NS = 6,
		Osi = 7,
		Pup = 4,
		Sna = 11,
		Unix = 1,
		Unknown = -1,
		Unspecified = 0,
		VoiceView = 0x12
	};
}

namespace SocketType
{
	enum Enum
	{
		Dgram = 2,
		Raw = 3,
		Rdm = 4,
		Seqpacket = 5,
		Stream = 1,
		Unknown = -1
	};
}

namespace ProtocolType
{
	enum Enum
	{
		Ggp = 3,
		Icmp = 1,
		IcmpV6 = 0x3a,
		Idp = 0x16,
		Igmp = 2,
		IP = 0,
		IPSecAuthenticationHeader = 0x33,
		IPSecEncapsulatingSecurityPayload = 50,
		IPv4 = 4,
		IPv6 = 0x29,
		IPv6DestinationOptions = 60,
		IPv6FragmentHeader = 0x2c,
		IPv6HopByHopOptions = 0,
		IPv6NoNextHeader = 0x3b,
		IPv6RoutingHeader = 0x2b,
		Ipx = 0x3e8,
		ND = 0x4d,
		Pup = 12,
		Raw = 0xff,
		Spx = 0x4e8,
		SpxII = 0x4e9,
		Tcp = 6,
		Udp = 0x11,
		Unknown = -1,
		Unspecified = 0
	};
}


namespace SelectMode
{
	enum Enum
	{
		SelectRead,
		SelectWrite,
		SelectError
	};
}


struct IPAdress
{
	uint64 adress;
	static IPAdress Any;
	static IPAdress Loopback;
	static IPAdress Broadcast;
	static IPAdress None;
	IPAdress(uint32 newAdress) {
		adress = newAdress;
	}
	IPAdress(const IPAdress& other) {
		adress = other.adress;
	}
	static IPAdress Parse(const char* str);
	std::string ToString() const;
};

struct IPEndPoint
{
	IPAdress adress;
	int32 port;
	IPEndPoint( ) : adress(IPAdress(0)), port(0) {}	
	IPEndPoint( const IPEndPoint& other) : adress(other.adress), port(other.port) { }
	IPEndPoint( const IPAdress& other, int nport  ) : adress(other), port(nport) {}
	IPEndPoint( uint64 adress, int nport) : adress(IPAdress(adress)), port(nport) { }
	std::string ToString() const;
};

struct IAsyncResult;
struct SocketIOManager 
{
	static SocketIOManager& Default();
protected:
	friend struct Socket;
	virtual IAsyncResult* BeginSend( uint8* buffer, int32 offset, int32 size, void* state ) = 0x0;
	virtual IAsyncResult* BeginReceive( uint8* buffer, int32 offset, int32 size, void* state ) = 0x0;	
	virtual int  EndSend( IAsyncResult* result ) = 0x0;
	virtual int  EndReceive( IAsyncResult* result ) = 0x0;	
};

struct IAsyncResult
{
	virtual SocketIOManager& Manager() = 0x0;
	virtual void* AsyncState() = 0x0;
	virtual bool IsCompleted() = 0x0;
};

struct Socket
{
	struct Impl;
	uint8 m_impl[12];

	void Accept(Socket& accepted);
	void Listen(int backlog);
	void Shutdown( int shutdownKinds );
	void Disconnect( bool reuseSocket );
	void Connect( const IPAdress& adress, int port);
	void Connect( const IPEndPoint& endPoint);	
	void Connect( const char* hostadress, int port );
	void Bind(const IPEndPoint& endPoint);
	bool Poll( int microSeconds, SelectMode::Enum mode);
	void Close( int timeout );
	void Close();

	bool Blocking();
	void Blocking(bool blocking);
	int  ReceiveTimeout();
	void ReceiveTimeout(int timeout);
	int  SendTimeout();
	void SendTimeout(int timeout);
	int  Send( uint8* buffer, int32 offset, int32 size );
	int  Receive( uint8* buffer, int32 offset, int32 size );
	IPEndPoint const* RemoteEndPoint(IPEndPoint& endPoint);
	IPEndPoint const* LocalEndPoint(IPEndPoint& endPoint);
	
	IAsyncResult*  BeginSend( uint8* buffer, int32 offset, int32 size, void* state, SocketIOManager& manager = SocketIOManager::Default() );
	IAsyncResult*  BeginReceive( uint8* buffer, int32 offset, int32 size, void* state, SocketIOManager& manager = SocketIOManager::Default() );	
	int  EndSend( IAsyncResult* result );
	int  EndReceive( IAsyncResult* result );	

	Socket();
	Socket(AdressFamilly::Enum familly, SocketType::Enum socketType, ProtocolType::Enum protocolType);
	~Socket();
};

struct TcpListener
{	
	struct Impl;
	uint8 m_impl[32];

	TcpListener(IPAdress& adress, int port);
	TcpListener(IPEndPoint& endPoint);
	~TcpListener();
	bool Pending();
	Socket Accept();
	void Start(int backlog);
	void Start();
	void Stop();
};


class SocketException : public std::runtime_error {
public:
	SocketException(const char* description) : std::runtime_error(description) { }
};
