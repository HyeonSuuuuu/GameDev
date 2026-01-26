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
}
