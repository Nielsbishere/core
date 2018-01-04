#pragma once

#include <Template/PlatformDefines.h>
#include <Types/Generic.h>
#include <Types/OString.h>
#include <Types/Buffer.h>

namespace oi {

	namespace sock {

		struct CS_IpAddress {
			u8 addr[4];
		};

		class ClientSocket {

			friend class BridgeClient;

		public:

			//Creates a ClientSocket which is bound to the address put in.
			//@param OString ip; the hostname or ip of the ServerSocket; example: osomi.net or 127.0.0.1 (/localhost)
			//@param u16 port; the port at which the ServerSocket is located
			//@return ClientSocket*; nullptr when it can't find the ip/hostname or can't reach.
			static ClientSocket *create(OString ip, u16 port);

			//Deconstructor, gets rid of socket.
			~ClientSocket();

			//@return current ip
			OString getIp();

			//@return current port
			u16 getPort();

			//@return whether or not the socket is open
			bool isOpen();

			//Output a string to the socket, if connected
			//@param Buffer data to be sent
			void output(Buffer b);

			//Gets the input from the ServerSocket
			//@optional u32 maxSize; the maximum amount of bytes it can receive. Set to 2kB by default.
			//@return string data that is received.
			//@note if you need more than 2kB or you don't know the amount of data you need, send a 'size' byte where you specify the length of the object sent.
			//		This can be a u32 which specifies the length in bytes.
			Buffer input(u32 maxSize = 2048);

		protected:

			void close();

		private:

			#ifdef __WINDOWS__
					SOCKET s;
					WSADATA wsa;
					SOCKADDR_IN server;
			#elif __UNIX__
					int s;
					sockaddr_in server;
			#endif

			OString ip;
			CS_IpAddress internIp;
			u16 port;
			bool open;

			ClientSocket(OString addr, u16 _port);
			OString translate(CS_IpAddress address);
		};

	}

}