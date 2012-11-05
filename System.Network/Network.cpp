#include "Network.h"

#if PLATFORM == PLATFORM_WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#elif PLATFORM == PLATFORM_LINUX 
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctime>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif 


namespace 
{
	const char* resolveError(int errorCode)
	{
		#if PLATFORM == PLATFORM_WIN32
		switch(errorCode)
		{
			case WSANOTINITIALISED:
				return "A successful WSAStartup call must occur before using this function.";
			case WSAENETDOWN:
				return "The network subsystem has failed.";
			case WSAEADDRINUSE:
				return "Only one usage of each socket address (protocol/network address/port) is normally permitted.";
			case WSAEINTR:
				return "The blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.";
			case WSAEINPROGRESS:
				return "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
			case WSAEALREADY:
				return "A nonblocking connect call is in progress on the specified socket.";
			case WSAEADDRNOTAVAIL:
				return "The remote address is not a valid address (such as INADDR_ANY or in6addr_any)";
			case WSAEAFNOSUPPORT:
				return "Addresses in the specified family cannot be used with this socket.";
			case WSAECONNREFUSED:
				return "The attempt to connect was forcefully rejected.";
			case WSAEFAULT:
				return "The sockaddr structure pointed to by the name contains incorrect address format for the associated address family or the namelen parameter is too small. This error is also returned if the sockaddr structure pointed to by the name parameter with a length specified in the namelen parameter is not in a valid part of the user address space.";
			case WSAEINVAL:
				return "The parameter s is a listening socket.";
			case WSAEISCONN:
				return "The socket is already connected (connection-oriented sockets only).";
			case WSAENETUNREACH:
				return "The network cannot be reached from this host at this time.";
			case WSAEHOSTUNREACH:
				return "A socket operation was attempted to an unreachable host.";
			case WSAENOBUFS:
				return "No buffer space is available. The socket cannot be connected.";
			case WSAENOTSOCK:
				return "The descriptor specified in the s parameter is not a socket.";
			case WSAETIMEDOUT:
				return "An attempt to connect timed out without establishing a connection.";
			case WSAEWOULDBLOCK:
				return "The socket is marked as nonblocking and the connection cannot be completed immediately.";
			case WSAEACCES:
				return "An attempt to connect a datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled.";
				
		}
		#endif 
		
		return 0x0;
	}

	#if PLATFORM == PLATFORM_WIN32
	struct NetworkScope
	{
		NetworkScope()
		{
			WSADATA wsaData;
			int error = WSAStartup( MAKEWORD(2, 2), &wsaData );
		}

		~NetworkScope()
		{
			WSACleanup();
		}
	
	} m_scope;
	#endif 
}

struct Socket::Impl
{
	SOCKET socket;	
	int adressFamilly : 24;
	int blocking	  :  8;
};

struct TcpListener::Impl
{
	IPEndPoint	endPoint;
	Socket		socket;	
	TcpListener::Impl(const Socket& s, const IPEndPoint& e) : socket(s), endPoint(e) { }
};




IPAdress IPAdress::Any(0);
IPAdress IPAdress::Loopback(0x100007f);
IPAdress IPAdress::Broadcast(0xffffffffL);
IPAdress IPAdress::None(0xffffffffL);
IPAdress IPAdress::Parse(const char* str)
{
	IPAdress adress(inet_addr(str));
	return adress;
}

std::string IPAdress::ToString() const
{
	sockaddr_in addr; 
	addr.sin_addr.s_addr = adress; 
	std::string str;
	char* ptr = inet_ntoa(addr.sin_addr);
	str.append( ptr );
	return str;
}

std::string IPEndPoint::ToString() const
{
	sockaddr_in addr; 
	addr.sin_addr.s_addr = adress.adress; 
	std::string str;
	char* ptr = inet_ntoa(addr.sin_addr);
	str.append( ptr );
	str.append( ":" );
	char buffer[32];
	sprintf(buffer, "%d", port);
	str.append( buffer );
	return str;
}





Socket::Socket()
{
	STATIC_ASSERT(sizeof(Impl) <= sizeof(m_impl));
	new (&m_impl) Impl();
	reinterpret_cast<Socket::Impl*>(&m_impl)->adressFamilly = AdressFamilly::Unspecified;
	reinterpret_cast<Socket::Impl*>(&m_impl)->socket = INVALID_SOCKET;
}
Socket::Socket(AdressFamilly::Enum familly, SocketType::Enum socketType, ProtocolType::Enum protocolType)
{
	STATIC_ASSERT(sizeof(Impl) <= sizeof(m_impl));
	new (&m_impl) Impl();
	reinterpret_cast<Socket::Impl*>(&m_impl)->adressFamilly = familly;
	reinterpret_cast<Socket::Impl*>(&m_impl)->socket = socket(familly, socketType, protocolType);
    if (reinterpret_cast<Socket::Impl*>(&m_impl)->socket == INVALID_SOCKET) {
        wprintf(L"socket function failed with error: %ld\n", WSAGetLastError());
    }
}

Socket::~Socket()
{

}




void Socket::Accept(Socket& accepted)
{	
	#if PLATFORM == PLATFORM_WIN32
	u_long nonblocking = 0;
	ioctlsocket(reinterpret_cast<Impl*>(&m_impl)->socket, FIONBIO, &nonblocking);
	#elif PLATFORM == PLATFORM_LINUX
	fcntl(s, F_SETFL, fcntl(reinterpret_cast<Impl*>(&m_impl)->socket, F_GETFL, 0) & ~O_NONBLOCK);
	#endif

	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX	
	reinterpret_cast<Impl*>(&accepted.m_impl)->socket = accept( reinterpret_cast<Impl*>(&m_impl)->socket, 0,0);
	if(reinterpret_cast<Impl*>(&accepted.m_impl)->socket == INVALID_SOCKET) {		
	}
	
	#endif	
}

void Socket::Shutdown( int shutdownKinds )
{	
	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX
	shutdown(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, shutdownKinds);
	#endif
}

void Socket::Disconnect( bool reuseSocket )
{
	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX	
	int result = reuseSocket == true ? 1 : 0;
	setsockopt(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, SOL_SOCKET, SO_REUSEADDR, (char*)&result, sizeof( result ));
	#endif
}

void Socket::Connect( const IPAdress& adress, int port)
{
	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX	
	sockaddr_in remote;
	remote.sin_family=reinterpret_cast<Socket::Impl*>(&m_impl)->adressFamilly; 
	remote.sin_addr.s_addr = adress.adress;
	remote.sin_port=htons(port); //port to use

	int error = 0;
	if( SOCKET_ERROR == (error = connect(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, (sockaddr*)&remote, sizeof(remote)))) {
		#if PLATFORM == PLATFORM_WIN32 
		int errorCode = WSAGetLastError();
		closesocket(reinterpret_cast<Socket::Impl*>(&m_impl)->socket);
		throw SocketException(resolveError(errorCode));			
		#elif
		closesocket(reinterpret_cast<Socket::Impl*>(&m_impl)->socket);
		throw SocketException(0);			
		#endif
	}

	#endif
}

void Socket::Connect( const IPEndPoint& endPoint)
{
	Connect(endPoint.adress, endPoint.port );
}

void Socket::Connect( const char* hostname, int port )
{	
	IPAdress adress(0);	
	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX	
	addrinfo* addr;
	if(getaddrinfo(hostname, 0, 0, &addr) != 0) {
		#if PLATFORM == PLATFORM_WIN32 
		int errorCode = WSAGetLastError();
		throw SocketException(resolveError(errorCode));		
		#elif
		throw SocketException(0);		
		#endif
		return;
	}
			
	struct sockaddr_in *sa = (struct sockaddr_in *) addr->ai_addr;
	adress.adress = sa->sin_addr.s_addr;
	freeaddrinfo(addr);		
	if(strcmp(hostname, "localhost") == 0)
		adress = IPAdress::Loopback;
	#endif

	Connect(adress, port);
}

void Socket::Bind(const IPEndPoint& endPoint)
{
	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX	
	sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_addr.s_addr=endPoint.adress.adress; 
	local.sin_port=htons(endPoint.port); 
	int error = bind(reinterpret_cast<Socket::Impl*>(&m_impl)->socket,(sockaddr*)&local, sizeof(local));
	if( error != 0 ) {
		#if PLATFORM == PLATFORM_WIN32 
		int errorCode = WSAGetLastError();
		throw SocketException(resolveError(errorCode));		
		#elif
		throw SocketException(0);		
		#endif		
	}
	#endif
}	

void Socket::Listen(int backlog)
{
	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX
	int error = listen(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, backlog);
	if( error != 0 ) {
		#if PLATFORM == PLATFORM_WIN32 
		int errorCode = WSAGetLastError();
		throw SocketException(resolveError(errorCode));		
		#elif
		throw SocketException(0);		
		#endif		
	}
	#endif
}


void Socket::Close( int timeout )
{
	if( timeout < 0 )
	{
		//close socket immediately
		#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX
		closesocket(reinterpret_cast<Socket::Impl*>(&m_impl)->socket);
		#endif
	}	
	else
	{
		//shutdown any send operations
		#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX
		shutdown(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, SD_SEND);
		#endif

		#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX
		//set the receive timeout 
        if( setsockopt(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != 0) {
			closesocket(reinterpret_cast<Socket::Impl*>(&m_impl)->socket);  
		}
		//await any data & close it
		else if( recv(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, 0, 0, 0) != 0 ) {
			closesocket(reinterpret_cast<Socket::Impl*>(&m_impl)->socket);  
		}
		else 
		{
			//Remove the async bit and close it.
			u_long bytesRead = 0;
			if((ioctlsocket(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, FIONREAD, &bytesRead) != 0) || (bytesRead != 0)) {
				closesocket(reinterpret_cast<Socket::Impl*>(&m_impl)->socket);  
			}
			//otherwise close the socket anyhow.
			else
			{
				closesocket(reinterpret_cast<Socket::Impl*>(&m_impl)->socket);  
			}
			
		}		
		#endif

	}
}

void Socket::Close()
{	
	Close(-1);
}

int  Socket::ReceiveTimeout()
{	
	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX
	uint32 timeout; int l = sizeof(timeout);
	if( getsockopt(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, &l) != -1) {
		return timeout;
	} else {
		return -1;
	}
	#endif
}

int  Socket::SendTimeout()
{
	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX
	uint32 timeout; int l = sizeof(timeout);
	if( getsockopt(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, &l) != -1) {
		return timeout;
	} else {
		return -1;
	}
	#endif
}
void Socket::ReceiveTimeout(int timeout)
{
    if (timeout < -1)
    {
       return;
    }
	if (timeout == -1)
	{
        timeout = 0;
    }

	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX
	setsockopt(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof( timeout ));
	#endif
}
void Socket::SendTimeout(int timeout)
{
    if (timeout < -1)
    {
       return;
    }
	if (timeout == -1)
	{
        timeout = 0;
    }

	#if PLATFORM == PLATFORM_WIN32 || PLATFORM == PLATFORM_LINUX
	setsockopt(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof( timeout ));
	#endif
}

int  Socket::Send( uint8* buffer, int32 offset, int32 size )
{
	int length = send(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, (char*)(buffer + offset), size, 0);
	if( length == SOCKET_ERROR ) {
		#if PLATFORM == PLATFORM_WIN32 
		int errorCode = WSAGetLastError();
		throw SocketException(resolveError(errorCode));		
		#elif
		throw SocketException(0);		
		#endif	
	}

	return length;
}

int  Socket::Receive( uint8* buffer, int32 offset, int32 size )
{
	int length = recv(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, (char*)(buffer + offset), size, 0);
	if( length == SOCKET_ERROR ) {
		#if PLATFORM == PLATFORM_WIN32 
		int errorCode = WSAGetLastError();
		throw SocketException(resolveError(errorCode));		
		#elif
		throw SocketException(0);		
		#endif	
	}

	return length;
}

bool Socket::Blocking()
{
	return reinterpret_cast<Socket::Impl*>(&m_impl)->blocking == 1;
}

void Socket::Blocking(bool blocking)
{
	int value = blocking == true ? 1 : 0;
	if( reinterpret_cast<Socket::Impl*>(&m_impl)->blocking != value ) {
		reinterpret_cast<Socket::Impl*>(&m_impl)->blocking = value;
		if( blocking == true )
		{
			//remove non-blocking status
			#if PLATFORM == PLATFORM_WIN32
			u_long nonblocking = 0;
			if( ioctlsocket(reinterpret_cast<Impl*>(&m_impl)->socket, FIONBIO, &nonblocking) != 0x0 ) {
				int errorCode = WSAGetLastError();
				throw SocketException(resolveError(errorCode));	
			}
			#elif PLATFORM == PLATFORM_LINUX
			fcntl(s, F_SETFL, fcntl(reinterpret_cast<Impl*>(&m_impl)->socket, F_GETFL, 0) & ~O_NONBLOCK);
			#endif
		}
		else
		{
			//add blocking status
			#if PLATFORM == PLATFORM_WIN32
			u_long nonblocking = 1;
			if( ioctlsocket(reinterpret_cast<Impl*>(&m_impl)->socket, FIONBIO, &nonblocking) != 0x0 ) { 
				int errorCode = WSAGetLastError();
				throw SocketException(resolveError(errorCode));	
			}
			#elif PLATFORM == PLATFORM_LINUX
			fcntl(s, F_SETFL, fcntl(reinterpret_cast<Impl*>(&m_impl)->socket, F_GETFL, 0) | O_NONBLOCK);
			#endif
		}
	}
}


bool Socket::Poll( int microSeconds, SelectMode::Enum mode)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, &fds);

	int num = 0;
    if (microSeconds != -1)
    {		
		timeval socketTime;
		socketTime.tv_sec = (int) (microSeconds / 0xf4240);
		socketTime.tv_usec = (int) (microSeconds % 0xf4240);		

		switch(mode)
		{
			case SelectMode::SelectRead:
				num = select(0, &fds, 0, 0, &socketTime);
				break;
			case SelectMode::SelectWrite:
				num = select(0, 0, &fds, 0, &socketTime);
				break;
			case SelectMode::SelectError:
				num = select(0, 0, 0, &fds, &socketTime);
				break;
		}		
    }
    else
    {
        switch(mode)
		{
			case SelectMode::SelectRead:
				num = select(0, &fds, 0, 0, 0);
				break;
			case SelectMode::SelectWrite:
				num = select(0, 0, &fds, 0, 0);
				break;
			case SelectMode::SelectError:
				num = select(0, 0, 0, &fds, 0);
				break;
		}
    }

	if (num == -1)
	{
		throw SocketException("Poll failed.");			
	}

	if ( fds.fd_count == 0x0 )
    {
        return false;
    }

	return fds.fd_array[0] == reinterpret_cast<Socket::Impl*>(&m_impl)->socket;

}



IPEndPoint const* Socket::LocalEndPoint(IPEndPoint& endPoint)
{
	sockaddr_in addr; int length = sizeof(sockaddr_in);
    if (getsockname(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, (sockaddr*)&addr, &length) == 0) {
	   endPoint = IPEndPoint(addr.sin_addr.s_addr, ntohs( addr.sin_port ));
	   return &endPoint;
    }

	return 0x0;
}

IPEndPoint const* Socket::RemoteEndPoint(IPEndPoint& endPoint)
{
	sockaddr_in addr; int length = sizeof(sockaddr_in);
    if (getpeername(reinterpret_cast<Socket::Impl*>(&m_impl)->socket, (sockaddr*)&addr, &length) == 0) {
	   endPoint = IPEndPoint(addr.sin_addr.s_addr, ntohs( addr.sin_port ));
	   return &endPoint;
    }

	return 0x0;
}




IAsyncResult*  Socket::BeginSend( uint8* buffer, int32 offset, int32 size, void* state, SocketIOManager& manager/* = SocketIOManager::Default()*/ )
{
	return manager.BeginSend(buffer,offset,size, state);
}	

IAsyncResult*  Socket::BeginReceive( uint8* buffer, int32 offset, int32 size, void* state, SocketIOManager& manager/* = SocketIOManager::Default()*/ )
{
	return manager.BeginReceive(buffer,offset,size, state);
}

int  Socket::EndSend( IAsyncResult* result )
{
	return result->Manager().EndSend( result );
}
int  Socket::EndReceive( IAsyncResult* result )
{
	return result->Manager().EndReceive( result );
}


TcpListener::TcpListener(IPAdress& adress, int port)
{
	STATIC_ASSERT(sizeof(Impl) <= sizeof(m_impl));
	new (&m_impl) Impl(
		Socket(AdressFamilly::InterNetwork, SocketType::Stream, ProtocolType::Tcp), 
		IPEndPoint(adress.adress, port)
	);
}

TcpListener::TcpListener(IPEndPoint& endPoint)
{
}

TcpListener::~TcpListener()
{
}

bool TcpListener::Pending()
{
	return reinterpret_cast<Impl*>(&m_impl)->socket.Poll(0, SelectMode::SelectRead);
}

void TcpListener::Start()
{
	Start(0x7fffffff);
}

Socket TcpListener::Accept()
{
	Socket r;
	reinterpret_cast<Impl*>(&m_impl)->socket.Accept(r);
	return r;
}

void TcpListener::Start(int backlog)
{
	if ((backlog > 0x7fffffff) || (backlog < 0))
    {
        throw SocketException("Argument backlog is out of range.");
    }

	reinterpret_cast<Impl*>(&m_impl)->socket.Bind(reinterpret_cast<Impl*>(&m_impl)->endPoint);
	reinterpret_cast<Impl*>(&m_impl)->socket.Listen(backlog);
}

void TcpListener::Stop()
{
	reinterpret_cast<Impl*>(&m_impl)->socket.Close();
}


struct NullIOManager : SocketIOManager
{
	IAsyncResult* BeginSend( uint8* buffer, int32 offset, int32 size, void* state ) 
	{	
		throw SocketException("Operation has not been implemented.");			
	}

	IAsyncResult* BeginReceive( uint8* buffer, int32 offset, int32 size, void* state ) 
	{
		throw SocketException("Operation has not been implemented.");			
	}
	int  EndSend( IAsyncResult* result )
	{
		throw SocketException("Operation has not been implemented.");			
	}
	int  EndReceive( IAsyncResult* result ) 
	{
		throw SocketException("Operation has not been implemented.");			
	}
};

SocketIOManager& SocketIOManager::Default()
{
	static NullIOManager manager;
	return manager;
}