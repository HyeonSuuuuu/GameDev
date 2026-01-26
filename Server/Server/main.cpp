import EchoServer;
import Common;

constexpr uint16 SERVER_PORT = 11021;
constexpr uint16 MAX_CLIENT = 100;


int main()
{
	EchoServer server;

	server.InitEnvironment();
	server.InitSocket();
	server.BindandListen(11021);
	server.Run(100);

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