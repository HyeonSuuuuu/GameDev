export module PacketData;
import <winSock2.h>;
import Common;


export class PacketData
{
public:
	PacketData() = delete;
	PacketData(uint32 sessionIndex, std::span<const byte> spanData)
		: m_sessionIndex{ sessionIndex }
		, m_dataSize{ static_cast<uint32>(spanData.size())}
	{
		m_packetData = new byte[m_dataSize];
		CopyMemory(m_packetData, spanData.data(), spanData.size());
	}
	~PacketData()
	{
		if (m_packetData != nullptr)
		{
			delete[] m_packetData;
			m_packetData = nullptr;
		}
		m_sessionIndex = 0;
		m_dataSize = 0;
	}
	PacketData(const PacketData&) = delete;
	PacketData& operator=(const PacketData&) = delete;
	PacketData(PacketData&& other) noexcept
	{
		m_sessionIndex = other.m_sessionIndex;
		m_dataSize = other.m_dataSize;
		m_packetData = other.m_packetData;
		other.m_packetData = nullptr;
		other.m_dataSize = 0;
		other.m_sessionIndex = 0;	
	}

	constexpr uint32 GetSessionIndex() const
	{
		return m_sessionIndex;
	}

	constexpr std::span<const byte> GetSpanData() const
	{
		return std::span<const byte>(m_packetData, m_dataSize);
	}
private:
	uint32 m_sessionIndex = 0;
	uint32 m_dataSize = 0;
	byte* m_packetData = nullptr;
};