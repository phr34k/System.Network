#include "Network.h"

void main()
{
	try
	{
		TcpListener l(IPAdress::Loopback, 3200);
		l.Start();

		Socket s(AdressFamilly::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
		s.Connect(IPAdress::Loopback, 3200);

		while( true )
		{
			Socket s;
			if( l.Pending() == true ) {
				s = l.Accept();
				
				IPEndPoint rpoint, lpoint;
				printf("accepted %s - %s\r\n", s.LocalEndPoint(lpoint)->ToString().c_str(),  s.RemoteEndPoint(rpoint)->ToString().c_str() );
			}
		}
	}
	catch( SocketException& s )
	{
		printf("error occured: %s\r\n", s.what());
	}
}