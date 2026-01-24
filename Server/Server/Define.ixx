export module Define;
import Common;
import <winSock2.h>;

export {
	constexpr int16 MAX_SOCKBUF = 1024;
	constexpr int8 MAX_WORKERTHREAD = 4;

	enum class IOOperation : uint8
	{
		RECV,
		SEND,
	};

	struct OverlappedEx
	{
		WSAOVERLAPPED m_wsaOverlapped;
		SOCKET m_socketClient;
		WSABUF m_wsaBuf;
		IOOperation m_operation;
	};

	struct ClientInfo
	{
		uint32 m_index = 0;
		SOCKET m_socketClient = INVALID_SOCKET;
		OverlappedEx m_recvOverlappedEx {};
		OverlappedEx m_sendOverlappedEx {};
		char m_sendBuf[MAX_SOCKBUF] {};
		char m_recvBuf[MAX_SOCKBUF] {};

		ClientInfo() = delete;
		ClientInfo(uint32 index)
			: m_index(index)
		{ }
	};
}
