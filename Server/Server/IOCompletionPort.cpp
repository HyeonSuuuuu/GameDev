module;
#include <winsock2.h>
#include <Ws2tcpip.h>

module IOCompletionPort;

import Common;

namespace IOCompletionPort
{
	ClientInfo::ClientInfo()
		: socketClient(INVALID_SOCKET)
	{
		ZeroMemory(&recvOverlappedEx, sizeof(OverlappedEx));
		ZeroMemory(&sendOverlappedEx, sizeof(OverlappedEx));
	}


	IOCompletionPort::IOCompletionPort()
		: m_listenSocket(INVALID_SOCKET)
		, m_clientCnt(0)
		, m_IOCPHandle(INVALID_HANDLE_VALUE)
		, m_isWorkerRun(true)
		, m_isAccepterRun(true)
		, m_socketBuf{}
	{
	}

	IOCompletionPort::~IOCompletionPort()
	{
		WSACleanup();
	}

	bool IOCompletionPort::InitEnvironment()
	{
		
		return true;
	}

	bool IOCompletionPort::InitSocket()
	{
		WSADATA wsaData;

		int32 ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (ret != 0)
		{
			std::print("[에러] WSAStartup() 실패 : {:d}\n", WSAGetLastError());
			return false;
		}

		m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (m_listenSocket == INVALID_SOCKET)
		{
			std::print("[에러] socket() 실패 : {:d}\n", WSAGetLastError());
		}
		
		std::print("소켓 초기화 성공\n");
		return true;
	}
}