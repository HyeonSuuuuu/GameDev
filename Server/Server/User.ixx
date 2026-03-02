export module User;
import Common;
import "../../Common/Protocol.h";
import <windows.h>;

export class User
{
	static constexpr uint32 PACKET_DATA_BUFFER_SIZE = 8096;

public:
	enum class DOMAIN_STATE
	{
		NONE = 0,
		CONNECTED = 1,
		LOGIN = 2,
		LOBBY = 3,
		ROOM = 4,
	};

	User() = delete;
	User(uint32 index)
	{
		m_index = index;
		m_packetDataBuffer = new byte[PACKET_DATA_BUFFER_SIZE];
	}
	~User()
	{
		delete[] m_packetDataBuffer;
	}

	void Init()
	{
		m_domainState = DOMAIN_STATE::CONNECTED;

		m_packetDataReadPos = 0;
		m_packetDataWritePos = 0;

		m_userId = "";
		m_roomIndex = std::nullopt;
	}

	void Clear()
	{
		m_roomIndex = std::nullopt;
		m_userId = "";
		m_domainState = DOMAIN_STATE::NONE;

		m_packetDataReadPos = 0;
		m_packetDataWritePos = 0;
	}
	
	int SetLogin(std::string_view userId)
	{
		m_domainState = DOMAIN_STATE::LOGIN;
		m_userId = userId;
		return 0;
	}

	void EnterRoom(uint32 roomIndex)
	{
		m_roomIndex = roomIndex;
		m_domainState = DOMAIN_STATE::ROOM;
	}

	void SetDomainState(DOMAIN_STATE value)
	{
		m_domainState = value;
	}

	std::optional<uint32> GetCurrentRoom() const
	{
		return m_roomIndex;
	}

	uint32 GetIndex() const
	{
		return m_index;
	}

	std::string GetUserId() const
	{
		return m_userId;
	}

	DOMAIN_STATE GetDomainState() const
	{
		return m_domainState;
	}

	void SetPacket(const std::span<byte> dataSpan)
	{
		if (dataSpan.size() + m_packetDataWritePos > PACKET_DATA_BUFFER_SIZE)
		{
			auto remainDataSize = m_packetDataWritePos - m_packetDataReadPos;
			if (remainDataSize > 0)
			{
				CopyMemory(m_packetDataBuffer, &m_packetDataBuffer[m_packetDataReadPos], remainDataSize);
				m_packetDataWritePos = remainDataSize;
			}
			else
			{
				m_packetDataWritePos = 0;
			}
			m_packetDataReadPos = 0;
		}
		CopyMemory(&m_packetDataBuffer[m_packetDataWritePos], dataSpan.data(), dataSpan.size());
		m_packetDataWritePos += static_cast<uint32>(dataSpan.size());
	}

	std::optional<Packet> GetPacket()
	{
		uint32 remainByte = m_packetDataWritePos - m_packetDataReadPos;
		
		if (remainByte < PACKET_HEADER_SIZE)
		{
			return std::nullopt;
		}

		auto pHeader = (PacketHeader*)&m_packetDataBuffer[m_packetDataReadPos];

		if (pHeader->size > remainByte)
		{
			return std::nullopt;
		}

		Packet packet;
		packet.sessionIndex = m_index;
		packet.packetId = pHeader->id;
		std::span<byte> span{ &m_packetDataBuffer[m_packetDataReadPos], pHeader->size };
		packet.dataSpan = span;

		m_packetDataReadPos += pHeader->size;
		return packet;
	}

private:
	uint32 m_index;
	std::optional<uint32> m_roomIndex;
	std::string m_userId;
	DOMAIN_STATE m_domainState;

	// ring buffer
	byte* m_packetDataBuffer;
	uint32 m_packetDataWritePos;
	uint32 m_packetDataReadPos;
};