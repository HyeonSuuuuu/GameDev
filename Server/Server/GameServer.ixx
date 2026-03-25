export module GameServer;

import IOCPNetwork;
import Common;
import Define;
import PacketManager;
import "../../Common/Protocol.h";



export class GameServer : public IOCPNetwork
{
public:
	GameServer() = default;
	virtual ~GameServer() = default;

	virtual void OnConnect(const uint32 clientIndex) override
	{
		
		bool ret = BindRecv(clientIndex);
		if (ret == false)
		{
			Log::Error("초기 Recv 등록 실패 (Index: {})", clientIndex);
			return;
		}
		std::print("[Onconnect]: Index({})\n", clientIndex);
		
		Packet packet{ clientIndex, (uint16)PACKET_ID::SYS_USER_CONNECT, {} };
		m_packetManager->EnqueSystemPacket(packet);
	}

	virtual void OnClose(const uint32 clientIndex) override
	{
		std::print("[OnClose]: Index({})\n", clientIndex);
		
		Packet packet{ clientIndex, (uint16)PACKET_ID::SYS_USER_DISCONNECT, {} };
		m_packetManager->EnqueSystemPacket(packet);
	}

	virtual void OnRecv(const uint32 clientIndex, const std::span<const byte> recvData) override
	{
		//std::print("[OnRecv]: Index({}), dataSize({})\n", clientIndex, recvData.size());
	
		m_packetManager->ReceivePacket(clientIndex, recvData);
	}

	void Run(const uint32 maxClient)
	{
		auto sendPacketFunc = [&](uint32 clientIndex, std::span<const byte> dataSpan)
			{
				SendMsg(clientIndex, dataSpan);
			};

		m_packetManager = std::make_unique<PacketManager>(maxClient);
		m_packetManager->m_sendFunc = sendPacketFunc;
		m_packetManager->Run();

		StartServer(maxClient);
	}

	void End()
	{
		m_packetManager->End();

		DestroyThread();
	}

private:
	std::unique_ptr<PacketManager> m_packetManager;
};