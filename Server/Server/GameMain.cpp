import GameServer;
import Common;
import "../../Common/Protocol.h";

constexpr uint16 SERVER_PORT = NetConfig::GAME_SERVER_PORT;
constexpr uint16 MAX_CLIENT = 10000;


int main()
{
	GameServer server;

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