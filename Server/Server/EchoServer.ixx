export module EchoServer;

import IOCPNetwork;
import Common;
import PacketData;
import Log;

export class EchoServer : public IOCPNetwork
{
public:
	EchoServer() = default;
	virtual ~EchoServer() = default;
	
	virtual void OnConnect(const uint32 clientIndex) override
	{
		bool ret = BindRecv(clientIndex);
		if (ret == false)
		{
			Log::Error("초기 Recv 등록 실패 (Index: {})", clientIndex);
			return;
		}
		std::print("[Onconnect]: Index({})\n", clientIndex);
	}
		
	virtual void OnClose(const uint32 clientIndex) override
	{
		std::print("[OnClose]: Index({})\n", clientIndex);
	}

	virtual void OnRecv(const uint32 clientIndex, const std::span<const char> recvData) override
	{
		//std::print("[OnRecv]: Index({}), dataSize({})\n", clientIndex, recvData.size());
	
		PacketData packet{ clientIndex, recvData };
		std::lock_guard<std::mutex> guard{ m_lock };
		m_packetDataQueue.emplace_back(std::move(packet));
	}

	void Run(const uint32 maxClient)
	{
		m_isRunProcessThread = true;
		m_processThread = std::thread([this]() { ProcessPacket(); });

		StartServer(maxClient);
	}

	void End()
	{
		m_isRunProcessThread = false;
		if (m_processThread.joinable())
		{
			m_processThread.join();
		}

		DestroyThread();
	}

private:
	void ProcessPacket()
	{
		while (m_isRunProcessThread)
		{
			auto packetOpt = DequePacketData();
			if (packetOpt.has_value())
			{
				std::span<const char>dataSpan{ packetOpt->GetSpanData()};
				SendMsg(packetOpt->GetSessionIndex(), dataSpan);
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}

	std::optional<PacketData> DequePacketData()
	{
		std::lock_guard<std::mutex> guard{ m_lock };
		if (m_packetDataQueue.empty())
		{
			return std::nullopt;
		}
		PacketData packetData{ std::move(m_packetDataQueue.front()) };
		m_packetDataQueue.pop_front();
		return std::move(packetData);
	}
	bool m_isRunProcessThread = false;
	std::thread m_processThread;
	std::mutex m_lock;
	std::deque<PacketData> m_packetDataQueue;
};
