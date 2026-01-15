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

	return 0;
}