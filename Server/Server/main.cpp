#include <print>

import EchoServer;
import Common;

constexpr uint16 SERVER_PORT = 11021;
constexpr uint16 MAX_CLIENT = 100;


int main()
{
	EchoServer server;

	server.InitEnvironment();
	server.InitSocket();
	server.BindandListen(SERVER_PORT);
	server.StartServer(MAX_CLIENT);
	std::print("아무 키나 누를 때까지 대기합니다\n");

	while (true)
	{
		std::string inputCmd;
		std::getline(std::cin, inputCmd);

		if (inputCmd == "quit")
		{
			break;
		}
	}

	server.DestroyThread();
	return 0;
}