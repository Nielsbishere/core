#include "Socket/ClientSocket.h"
#include <Utils/Log.h>

using namespace oi::sock;
using namespace oi;

ClientSocket *ClientSocket::create(OString ip, u16 port) {
	ClientSocket *s = new ClientSocket(ip, port);
	if (!s->isOpen()) {
		delete s;
		s = nullptr;
	}
	return s;
}

u16 ClientSocket::getPort() { return port; }
bool ClientSocket::isOpen() { return open; }
OString ClientSocket::getIp() { return ip; }

OString ClientSocket::translate(CS_IpAddress ip) {
	std::stringstream ss;
	ss << (u16)ip.addr[0] << "." << (u16)ip.addr[1] << "." << (u16)ip.addr[2] << "." << (u16)ip.addr[3];
	return ss.str();
}

ClientSocket::~ClientSocket() {
	close();
}

void ClientSocket::close() {

	if (open) {
		open = false;

		#ifdef _WIN32
		closesocket(s);
		WSACleanup();
		#elif __UNIX__
		close(s);
		#endif
	}

}

void ClientSocket::output(Buffer dat) {
	if (open) {

		int bytes = (int)dat.size(), size;

		while(bytes > 0){

			#ifdef _WIN32
			if ((size = send(s, (const char*)&dat[0], dat.size(), 0)) <= 0) {
			#elif __UNIX__
			if ((size = write(s, dat.getPtr(), dat.size())) <= 0) {
			#endif

				Log::error("Couldn't ouput to the socket!");
				close();
				break;
			}

			bytes -= size;
		}
	}
	else Log::warn("Couldn't ouput to the socket! It was already closed!");
}

Buffer ClientSocket::input(u32 maxSize) {
	if (open) {

		int size, end = 0, disp = 0, req = (int)maxSize;
		char *res = new char[maxSize];

		while(req > 0){
			#ifdef _WIN32
			if ((size = recv(s, res + disp, req, 0)) == SOCKET_ERROR) {
			#elif __UNIX__
			if ((size = read(s, res + disp, req)) < 0) {
			#endif

				Log::error("Couldn't input from the socket!");
				close();
				delete[] res;
				return Buffer(nullptr, 0);
			}

			if (size == 0) {
				end = disp;

				if (end == 0) {
					Log::error("Couldn't input from the socket!");
					close();
					delete[] res;
					return Buffer(nullptr, 0);
				}

				break;
			}

			disp += size;
			req -= size;
		}

		if (end == 0)
			end = (int)maxSize;

		Buffer b = Buffer((u8*)res, end);
		delete[] res;
		return b;
	}
	else {
		Log::warn("Couldn't get input from the socket! The socket was already closed!");
		return Buffer(nullptr, 0);
	}
}

ClientSocket::ClientSocket(OString addr, u16 _port) {

	ip = addr;
	port = _port;

	Log::println(OString("Connecting to ") + addr);

	#ifdef _WIN32
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			Log::error("Couldn't init winsock!");
			open = false;
		}
		else {
			if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
				Log::error("Couldn't open socket!");
				open = false;
			}
			else {
				server.sin_family = AF_INET;
				server.sin_port = htons(port);

				struct addrinfo hints, *res;
				memset(&hints, 0, sizeof(hints));
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_family = AF_INET;

				if (getaddrinfo(addr.c_str(), NULL, &hints, &res) != 0) {
					Log::error("Couldn't find that address!");
					open = false;
					return;
				}

				server.sin_addr.S_un = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.S_un;
				if (connect(s, (SOCKADDR*)&server, sizeof(server)) < 0) {
					Log::error("Couldn't connect to address!");
					open = false;
				}
				else {
					CS_IpAddress ip;
					memcpy(&ip, &server.sin_addr.S_un, 4);
					internIp = ip;
					Log::println(OString("Successfully connected the socket! (") + translate(ip) + ")");
					open = true;
				}
			}
		}
	#elif __UNIX__
		if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			Log::error("Couldn't open socket!");
			open = false;
		}
		else {
			struct hostent *host = gethostbyname(ip.c_str());
			if (host != nullptr) {
				Log::println(OString("Found host ") + addr);
				CS_IpAddress addr;
				memcpy(&addr, host->h_addr_list, 4);
				addr = translate(addr);
				internIp = addr;
			}
			bzero((char *)&server, sizeof(server));
			server.sin_family = AF_INET;
			bcopy((char *)host->h_addr, (char *)&server.sin_addr.s_addr, host->h_length);
			server.sin_port = htons(port);
			if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
				Log::error("Couldn't connect to address!");
				open = false;
			}
			else {
				Log::println("Successfully connected the socket!");
				open = true;
			}
		}
	#else
	#endif
}