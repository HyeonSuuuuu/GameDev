module;
#include <winSock2.h>

export module Define;
import Common;

export
{
	constexpr int16 MAX_SOCKBUF = 1024;
	constexpr int8 MAX_WORKERTHREAD = 4;
	constexpr int64 REUSE_WAIT_MS = 2000;

	enum class IOOperation : uint8
	{
		RECV,
		SEND,
		ACCEPT,
	};

	struct OverlappedEx : public OVERLAPPED
	{
		SOCKET m_socketClient;
		WSABUF m_wsaBuf;
		IOOperation m_operation;
		uint32 m_sessionIndex = 0;
	};

	struct Packet
	{
		uint32 sessionIndex;
		uint16 packetId;
		std::span<const byte> dataSpan;
	};
}