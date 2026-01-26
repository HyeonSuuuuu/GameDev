export module IOCPNetwork;
import <winsock2.h>;
import <ws2tcpip.h>;
import Common;
import Define;
import Log;
import Session;
#pragma comment(lib, "ws2_32.lib")

export class IOCPNetwork
{
public:
	IOCPNetwork() = default;
	virtual ~IOCPNetwork()
	{
		WSACleanup();
	}
		
	bool InitEnvironment()
	{
		return true;
	}
	bool InitSocket()
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
	bool BindandListen(const int32 port) const
	{
		sockaddr_in serverAddr {};
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
	bool StartServer(const uint32 maxClientCount)
	{
		CreateClient(maxClientCount);

		m_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, MAX_WORKERTHREAD);
		if (m_iocpHandle == nullptr)
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
	void DestroyThread()
	{
		m_isWorkerRun = false;
		CloseHandle(m_iocpHandle);

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

	bool SendMsg(const uint32 clientIndex, const std::span<const char> msg)
	{
		return m_sessions[clientIndex].SendMsg(msg);
	}

	virtual void OnConnect(const uint32 clientIndex) = 0;
	virtual void OnClose(const uint32 clientIndex) = 0;
	virtual void OnRecv(const uint32 clientIndex, const std::span<const char> data) = 0;


private:
	void CreateClient(const uint32 maxClientCount)
	{
		m_sessions.reserve(maxClientCount);
		for (uint32 i = 0; i < maxClientCount; ++i)
		{
			m_sessions.emplace_back(i);
		}
	}
	bool CreateWorkerThread()
	{
		uint8 threadId = 0;
		for (int i = 0; i < MAX_WORKERTHREAD; ++i)
		{
			m_IOWorkerThreads.emplace_back([this]() { WorkerThread(); });
		}

		Log::Success("{}개의 Worker Thread 생성 완료", MAX_WORKERTHREAD);
		return true;
	}
	bool CreateAccepterThread()
	{
		m_accepterThread = std::thread([this]() { AccepterThread(); });

		Log::Info("AccepterThread 생성 완료");
		return true;
	}
	Session* GetEmptySession()
	{
		for (auto& session : m_sessions)
		{
			if (session.isConnected())
			{
				return &session;
			}
		}
		return nullptr;
	}

	Session* GetSession(const uint32 sessionIndex)
	{
		if (sessionIndex >= m_sessions.size())
		{
			return nullptr;
		}
		return &m_sessions[sessionIndex];
	}
	
	void WorkerThread()
	{
		Session* pSession = nullptr;
		bool isSuccess = true;
		DWORD dwIoSize = 0;
		LPOVERLAPPED lpOverlapped = nullptr;

		while (m_isWorkerRun)
		{
			isSuccess = GetQueuedCompletionStatus(m_iocpHandle,
				&dwIoSize,
				(PULONG_PTR)&pSession,
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
				CloseSocket(pSession);
				continue;
			}

			OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped;

			if (IOOperation::RECV == pOverlappedEx->m_operation)
			{
				const auto dataSpan{ pSession->GetRecvData(dwIoSize) };
				OnRecv(pSession->GetIndex(), dataSpan);
				pSession->BindRecv();
			}
			else if (IOOperation::SEND == pOverlappedEx->m_operation)
			{
				delete[] pOverlappedEx->m_wsaBuf.buf;
				delete pOverlappedEx;
				std::print("[송신] bytes : {:d}\n ", dwIoSize);
			}
			else
			{
				Log::Warning("socket({}에서 예외상황", pSession->GetSocketId());
			}
		}
	}
	void AccepterThread()
	{
		sockaddr_in clientAddr{};
		int32 addrLen = sizeof(sockaddr_in);

		while (m_isAccepterRun)
		{
			SOCKET clientSocket = accept(m_listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket == INVALID_SOCKET)
			{
				Log::Error("accept() 함수 실패: {}", WSAGetLastError());
				continue;
			}
			Session* pSession = GetEmptySession();
			if (pSession == nullptr)
			{
				Log::Warning("Client Full");
				return;
			}
			
			pSession->OnConnect(m_iocpHandle, clientSocket);

			OnConnect(pSession->GetIndex());
			++m_clientCnt;
		}
	}
	void CloseSocket(Session* pSession, bool isForce = false)
	{
		pSession->Close(isForce);

		OnClose(pSession->GetIndex());
		--m_clientCnt;
	}

	std::vector<Session> m_sessions;
	SOCKET m_listenSocket = INVALID_SOCKET;
	int m_clientCnt = 0;
		
	std::vector<std::thread> m_IOWorkerThreads;
	std::thread m_accepterThread;
	HANDLE m_iocpHandle = INVALID_HANDLE_VALUE;
	bool m_isWorkerRun = true;
	bool m_isAccepterRun = true;
	char m_socketBuf[1024] {};
};