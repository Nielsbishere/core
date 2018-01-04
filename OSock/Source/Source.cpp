#include "OSockDependencies.h"
#include "Socket/ClientSocket.h"

using namespace oi::sock;
using namespace oi;

int main() {

	ClientSocket *cs = ClientSocket::create("google.nl", 80);

	if (cs != nullptr) {


		Buffer message = Buffer(OString("GET / HTTP/1.1\r\nHost: ") + cs->getIp() + "\r\nConnection: close\r\n\r\n");
		cs->output(message);
		message.deconstruct();

		Buffer b = cs->input();
		printf("%s\n", &b[0]);
		b.deconstruct();

	}

	system("pause");
	return 0;
}