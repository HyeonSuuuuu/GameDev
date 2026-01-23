module;
#include <winsock2.h>
#include <Ws2tcpip.h>

module IOCPNetwork;
import Log;


IOCPNetwork::~IOCPNetwork()
{
	WSACleanup();
}

bool IOCPNetwork::InitEnvironment()
{

	return true;
}

bool IOCPNetwork::InitSocket()
{
	WSADATA wsaData;

	int32 ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		Log::Error("WSAStartup() 실패 : {}", WSAGetLastError());
		return false;
	}

	m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (m_listenSocket == INVALID_SOCKET)
	{
		Log::Error("socket() 실패 : {}", WSAGetLastError());
	}

	Log::Success("소켓 초기화 성공");
	return true;
}

bool IOCPNetwork::BindandListen(const int32 port)
{
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int32 ret = bind(m_listenSocket, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR_IN));
	if (ret != 0)
	{
		Log::Error("bind() 실패 : {}", WSAGetLastError());
		return false;
	}

	ret = listen(m_listenSocket, 5);
	if (ret != 0)
	{
		Log::Error("listen() 실패 : {}", WSAGetLastError());
		return false;
	}

	Log::Success("서버 등록 성공");
	return true;
}

bool IOCPNetwork::StartServer(const uint32 maxClientCount)
{
	CreateClient(maxClientCount);

	m_IOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, MAX_WORKERTHREAD);
	if (m_IOCPHandle == nullptr)
	{
		Log::Error("CreateIoCompletionPort() 실패: {}", GetLastError());
		return false;
	}

	bool ret = CreateWorkerThread();
	if (ret == false)
	{
		return false;
	}

	ret = CreateAccepterThread();
	if (ret == false)
	{
		return false;
	}

	Log::Success("서버 시작");
	return true;
}

void IOCPNetwork::DestroyThread()
{
	m_isWorkerRun = false;
	CloseHandle(m_IOCPHandle);

	for (auto& th : m_IOWorkerThreads)
	{
		if (th.joinable())
		{
			th.join();
		}
	}

	m_isAccepterRun = false;
	closesocket(m_listenSocket);
		
	if (m_accepterThread.joinable())
	{
		m_accepterThread.join();
	}
}


void IOCPNetwork::CreateClient(const uint32 maxClientCount)
{
	m_clientInfos.reserve(maxClientCount);
	for (uint32 i = 0; i < maxClientCount; ++i)
	{
		m_clientInfos.emplace_back(i);
	}
}

bool IOCPNetwork::CreateWorkerThread()
{
	uint8 threadId = 0;
	for (int i = 0; i < MAX_WORKERTHREAD; ++i)
	{
		m_IOWorkerThreads.emplace_back([this]() { WorkerThread(); });
	}

	Log::Success("{}개의 Worker Thread 생성 완료", MAX_WORKERTHREAD);
	return true;
}

bool IOCPNetwork::CreateAccepterThread()
{
	m_accepterThread = std::thread([this]() { AccepterThread(); });
		
	Log::Info("AccepterThread 생성 완료");
	return true;
}

void IOCPNetwork::WorkerThread()
{
	ClientInfo* pClientInfo = nullptr;
	bool isSuccess = true;
	DWORD dwIoSize = 0;
	LPOVERLAPPED lpOverlapped = nullptr;
		
	while (m_isWorkerRun)
	{
		isSuccess = GetQueuedCompletionStatus(m_IOCPHandle,
			&dwIoSize,
			(PULONG_PTR)&pClientInfo,
			&lpOverlapped,
			INFINITE);

		if (isSuccess && dwIoSize == 0 && lpOverlapped == nullptr)
		{
			m_isWorkerRun = false;
			continue;
		}

		if (lpOverlapped == nullptr)
		{
			Log::Error("GetQueuedCompletionStatus() 실패: {}", GetLastError());
			continue;
		}

		if (!isSuccess || (dwIoSize == 0 && isSuccess))
		{
			CloseSocket(pClientInfo);
			continue;
		}

		OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped;

		if (IOOperation::RECV == pOverlappedEx->m_operation)
		{
			const std::span<char> recvData{ pClientInfo->m_recvBuf, dwIoSize };
			OnRecv(pClientInfo->m_index, recvData);
				
			SendMsg(pClientInfo, pClientInfo->m_recvBuf, dwIoSize);
			BindRecv(pClientInfo);
		}
		else if (IOOperation::SEND == pOverlappedEx->m_operation)
		{
			std::string_view msg(pClientInfo->m_sendBuf, dwIoSize);
			std::print("[송신] bytes : {:d} , msg : {:s}\n ", dwIoSize, msg);
		}
		else
		{
			Log::Warning("socket({:d}에서 예외상황", (int)pClientInfo->m_socketClient);
		}
	}
		
}

void IOCPNetwork::AccepterThread()
{
	sockaddr_in clientAddr;
	int32 addrLen = sizeof(sockaddr_in);
		
	while (m_isAccepterRun)
	{
		ClientInfo* pClientInfo = GetEmptyClientInfo();
		if (pClientInfo == nullptr)
		{
			Log::Warning("Client Full");
			return;
		}
			
		pClientInfo->m_socketClient = accept(m_listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (pClientInfo->m_socketClient == INVALID_SOCKET)
		{
			Log::Error("accept() 함수 실패: {}", WSAGetLastError());
			continue;
		}

		bool ret = BindIOCompletionPort(pClientInfo);
		if (ret == false)
		{
			Log::Error("IOCP 바인딩 실패 (Index: {})", pClientInfo->m_index);
			return;
		}
			
		ret = BindRecv(pClientInfo);
		if (ret == false)
		{
			Log::Error("초기 Recv 등록 실패 (Index: {})", pClientInfo->m_index);
			return;
		}

		char clientIP[INET_ADDRSTRLEN] {};
		inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, sizeof(clientIP));
		Log::Success("클라이언트 접속 : IP{:s} SOCKET({:d})", clientIP, pClientInfo->m_socketClient);

		OnConnect(pClientInfo->m_index);
		++m_clientCnt;
	}
}

void IOCPNetwork::CloseSocket(ClientInfo* pClientInfo, bool isForce)
{
	linger lingerOpt{ 0, 0 };

	if (isForce)
	{
		lingerOpt.l_onoff = 1;
	}
		
	shutdown(pClientInfo->m_socketClient, SD_BOTH);

	setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&lingerOpt, sizeof(lingerOpt));

	closesocket(pClientInfo->m_socketClient);
	pClientInfo->m_socketClient = INVALID_SOCKET;

	OnClose(pClientInfo->m_index);
}

bool IOCPNetwork::BindIOCompletionPort(ClientInfo* pClientInfo)
{
	auto hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient,
		m_IOCPHandle,
		(ULONG_PTR)pClientInfo,
		0);
		
	if (hIOCP == nullptr || m_IOCPHandle != hIOCP)
	{
		Log::Error("CreateCompletionPort() 실패: {}", GetLastError());
		return false;
	}

	return true;
}

bool IOCPNetwork::BindRecv(ClientInfo* pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	pClientInfo->m_recvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
	pClientInfo->m_recvOverlappedEx.m_wsaBuf.buf = pClientInfo->m_recvBuf;
	pClientInfo->m_recvOverlappedEx.m_operation = IOOperation::RECV;

	int ret = WSARecv(pClientInfo->m_socketClient,
		&(pClientInfo->m_recvOverlappedEx.m_wsaBuf),
		1,
		&dwRecvNumBytes,
		&dwFlag,
		(LPWSAOVERLAPPED) & (pClientInfo->m_recvOverlappedEx),
		nullptr);

	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		Log::Error("WSARecv() 실패 : {}", WSAGetLastError());
		return false;
	}

	return true;
}
bool IOCPNetwork::SendMsg(ClientInfo* pClientInfo, char* pMsg, int len)
{
	DWORD dwRecvNumBytes = 0;
		
	CopyMemory(pClientInfo->m_sendBuf, pMsg, len);

	pClientInfo->m_sendOverlappedEx.m_wsaBuf.len = len;
	pClientInfo->m_sendOverlappedEx.m_wsaBuf.buf = pClientInfo->m_sendBuf;
	pClientInfo->m_sendOverlappedEx.m_operation = IOOperation::SEND;

	int ret = WSASend(pClientInfo->m_socketClient,
		&(pClientInfo->m_sendOverlappedEx.m_wsaBuf),
		1,
		&dwRecvNumBytes,
		0,
		(LPWSAOVERLAPPED) & (pClientInfo->m_sendOverlappedEx),
		nullptr);

	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		Log::Error("WSASend() 실패 : {}", WSAGetLastError());
		return false;
	}
	return true;
}

ClientInfo* IOCPNetwork::GetEmptyClientInfo()
{
	for (auto& client : m_clientInfos)
	{
		if (client.m_socketClient == INVALID_SOCKET)
		{
			return &client;
		}
	}
	return nullptr;
}

