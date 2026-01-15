#include <print>

import IOCompletionPort;
import Common;

constexpr uint16 SERVER_PORT = 11021;
constexpr uint16 MAX_CLIENT = 100;

int main()
{
	IOCompletionPort::IOCompletionPort iocp;

	iocp.InitEnvironment();
	iocp.InitSocket();
	iocp.BindandListen(SERVER_PORT);
	iocp.StartServer(MAX_CLIENT);
	std::print("아무 키나 누를 때까지 대기합니다\n");
	getchar();

	iocp.DestroyThread();
	return 0;
}