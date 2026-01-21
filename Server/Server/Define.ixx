module;
#include <winsock2.h>
#include <ws2tcpip.h>

export module Define;

import Common;


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
		SOCKET m_socketClient;
		OverlappedEx m_recvOverlappedEx;
		OverlappedEx m_sendOverlappedEx;
		char m_sendBuf[MAX_SOCKBUF];
		char m_recvBuf[MAX_SOCKBUF];
		ClientInfo()
			: m_socketClient(INVALID_SOCKET)
			, m_sendBuf {}
			, m_recvBuf {}
		{
			ZeroMemory(&m_recvOverlappedEx, sizeof(OverlappedEx));
			ZeroMemory(&m_sendOverlappedEx, sizeof(OverlappedEx));
		}
	};
}
