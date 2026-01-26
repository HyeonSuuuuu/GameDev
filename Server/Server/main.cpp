import EchoServer;
import Common;

constexpr uint16 SERVER_PORT = 11021;
constexpr uint16 MAX_CLIENT = 10000;


int main()
{
	EchoServer server;

	server.InitEnvironment();
	server.InitSocket();
	server.BindandListen(SERVER_PORT);
	server.Run(MAX_CLIENT);

	while (true)
	{
		std::string inputCmd;
		std::getline(std::cin, inputCmd);

		if (inputCmd == "quit")
		{
			break;
		}
	}

	server.End();
	return 0;
}