export module PacketManager;

import Common;
import "../../Common/Protocol.h";
import UserManager;
import Define;

export class PacketManager
{
public:
	PacketManager() = delete;
	~PacketManager() = default;

	PacketManager(const uint32 maxClient)
	{
		Register(PACKET_ID::SYS_USER_CONNECT, &PacketManager::ProcessUserConnect);
		Register(PACKET_ID::SYS_USER_DISCONNECT, &PacketManager::ProcessUserDisConnect);
		Register(PACKET_ID::CS_LOGIN_REQUEST, &PacketManager::ProcessLogin);
		CreateComponent(maxClient);
	}

	bool Run()
	{
		Log::Success("패킷 처리 스레드 생성 성공");
		m_isRunProcessThread = true;
		m_processThread = std::thread([this]() { ProcessPacket(); });
		return true;
	}

	void End()
	{
		m_isRunProcessThread = false;
		if (m_processThread.joinable())
		{
			m_processThread.join();
		}
	}

	void ReceivePacket(const uint32 sessionIndex, const std::span<const byte> dataSpan)
	{
		auto pUser = m_userManager->GetUser(sessionIndex);
		pUser->SetPacket(dataSpan);
		EnqueLogicPacket(sessionIndex);
	}

	void ProcessPacket()
	{
		while (m_isRunProcessThread)
		{
			bool isIdle = true;
			if (std::optional<Packet> packetOpt = DequeLogicPacket(); packetOpt.has_value())
			{
				if (auto& packet = packetOpt.value(); packet.packetId > static_cast<uint16>(PACKET_ID::SYS_END))
				{
					isIdle = false;
					ProcessRecvPacket(packet.sessionIndex, packet.packetId, packet.dataSpan);
				}
			}
			
			if (std::optional<Packet> packetOpt = DequeSystemPacket(); packetOpt.has_value())
			{
				if (auto& packet = packetOpt.value(); packet.packetId != 0)
				{
					isIdle = false;
					ProcessRecvPacket(packet.sessionIndex, packet.packetId, packet.dataSpan);
				}
			}

			if (isIdle)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			
		}
	}
	void EnqueLogicPacket(const uint32 sessionIndex)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_logicUserIdxQueue.push_back(sessionIndex);
	}

	std::optional<Packet> DequeLogicPacket()
	{
		uint32 userIndex = 0;
		{
			std::lock_guard<std::mutex> guard(m_lock);
			if (m_logicUserIdxQueue.empty())
			{
				return std::nullopt;
			}
			userIndex = m_logicUserIdxQueue.front();
			m_logicUserIdxQueue.pop_front();
		}

		User* pUser = m_userManager->GetUser(userIndex);
		std::optional<Packet> packet = pUser->GetPacket();
		if (packet.has_value())
		{
			packet.value().sessionIndex = userIndex;
		}
		return packet;
	}

	void EnqueSystemPacket(Packet packet)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_systemPacketQueue.push_back(std::move(packet));
	}

	std::optional<Packet> DequeSystemPacket()
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if (m_systemPacketQueue.empty())
		{
			return std::nullopt;
		}

		auto packet = std::move(m_systemPacketQueue.front());
		m_systemPacketQueue.pop_front();

		return packet;
	}

	std::function<void(uint32 clientIndex, std::span<const byte> dataSpan)> m_sendFunc;
private:

	void CreateComponent(const uint32 maxClient)
	{
		m_userManager = std::make_unique<UserManager>(maxClient);
	}

	void ProcessRecvPacket(const uint32 sessionIndex, const uint16 packetId, std::span<const byte> packet)
	{
		if (auto it = m_recvFuncDict.find(packetId); it != m_recvFuncDict.end())
		{
			it->second(sessionIndex, packet);
		}
		else
		{
			Log::Warning("sessionId({}): 알 수 없는 패킷, packetId: {}", sessionIndex, packetId);
		}
	}

	void Register(PACKET_ID id, void(PacketManager::*handler)(uint32, std::span<const byte>))
	{
		m_recvFuncDict[static_cast<uint16>(id)] = [this, handler](uint32 idx, std::span<const byte> data) {
			(this->*handler)(idx, data);
		};
	}

	// --------------------------------- Packet Func -------------------------------------------------------------------

	void ProcessUserConnect(uint32 sessionIndex, std::span<const byte> dataSpan)
	{
		Log::Info("[ProcessUserConnect] sessionIndex: {}", sessionIndex);
		m_userManager->AddUser(sessionIndex);
		
	}

	void ProcessUserDisConnect(uint32 sessionIndex, std::span<const byte> dataSpan)
	{
		Log::Info("[ProcessUserDisConnect] sessionIndex: {}", sessionIndex);
		m_userManager->RemoveUser(sessionIndex);
	}
	
	void ProcessLogin(uint32 sessionIndex, std::span<const byte> dataSpan)
	{
		if (CS_LOGIN_PACKET_SIZE != dataSpan.size())
			return;

		const CS_Login* loginReqPacket = reinterpret_cast<const CS_Login*>(dataSpan.data());
		Log::Info("id: {} pw: {}", loginReqPacket->userID, loginReqPacket->userPW);

		SC_Login loginResPacket;
		
		
		
		// 이름 중복 검사
		
		Log::Info("로그인 요청");
	}

	std::function<void(uint32, std::span<const byte>)> m_recvFunc;
	std::unordered_map<uint16, std::function<void(uint32, std::span<const byte>)>> m_recvFuncDict;

	std::unique_ptr<UserManager> m_userManager;
	
	bool m_isRunProcessThread = false;
	std::thread m_processThread;
	std::mutex m_lock;

	std::deque<uint32> m_logicUserIdxQueue;
	std::deque<Packet> m_systemPacketQueue;

};