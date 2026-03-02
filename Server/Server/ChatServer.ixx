import IOCPNetwork;
import Common;
import PacketManager;
import "../../Common/Protocol.h";



export class ChatServer : public IOCPNetwork
{
public:
	ChatServer() = default;
	virtual ~ChatServer() = default;

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
		
	}

	virtual void OnClose(const uint32 clientIndex) override
	{
		std::print("[OnClose]: Index({})\n", clientIndex);
	}

	virtual void OnRecv(const uint32 clientIndex, const std::span<const byte> recvData) override
	{
		//std::print("[OnRecv]: Index({}), dataSize({})\n", clientIndex, recvData.size());
	}

	void Run(const uint32 maxClient)
	{
		auto sendPacketFunc = [&](uint32 clientIndex, std::span<const byte> dataSpan)
			{
				SendMsg(clientIndex, dataSpan);
			};
		StartServer(maxClient);
	}

	void End()
	{

		DestroyThread();
	}

private:
	std::unique_ptr<PacketManager> m_packetManager;
};