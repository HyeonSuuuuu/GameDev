export module IOCPNetwork;
import <winsock2.h>;
import <ws2tcpip.h>;
import Common;
import Define;
import Session;
import SessionManager;
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
		// 간혹 서버 주고받는 데이터가 utf-8이 아니라 cp-949임. 윈10 구버전에서 그러는거 같음
		std::locale::global(std::locale("ko_KR.UTF-8"));
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

		ret = listen(m_listenSocket, SOMAXCONN);
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

		HANDLE hIOCP = CreateIoCompletionPort((HANDLE)m_listenSocket, m_iocpHandle, (ULONG_PTR)0, 0);
		if (hIOCP == nullptr || hIOCP != m_iocpHandle)
		{
			Log::Error("ListenSocket IOCP 등록 실패 : {}", GetLastError());
			return false;
		}

		bool ret = CreateWorkerThread();
		if (ret == false)
		{
			return false;
		}

		for (int i = 0; i < 10; ++i)
		{
			Session* session = m_sessionManager->GetEmptySession();
			if (session == nullptr)
			{
				Log::Error("초기 세션 부족");
				break;
			}
			if (session->PostAccept(m_listenSocket) == false)
			{
				m_sessionManager->ReturnSession(session);
			}
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

		closesocket(m_listenSocket);
	}

	bool SendMsg(const uint32 clientIndex, const std::span<const char> msg)
	{
		return m_sessionManager->SendPacket(clientIndex, msg);
	}

	virtual void OnConnect(const uint32 clientIndex) = 0;
	virtual void OnClose(const uint32 clientIndex) = 0;
	virtual void OnRecv(const uint32 clientIndex, const std::span<const char> data) = 0;


protected:
	bool BindRecv(uint32 index)
	{
		return m_sessionManager->BindRecv(index);
	}

private:

	void CreateClient(const uint32 maxClientCount)
	{
		m_sessionManager = std::make_unique<SessionManager>(maxClientCount);
	}
	bool CreateWorkerThread()
	{
		m_isWorkerRun = true;
		for (int i = 0; i < MAX_WORKERTHREAD; ++i)
		{
			m_IOWorkerThreads.emplace_back([this]() { WorkerThread(); });
		}

		Log::Success("{}개의 Worker Thread 생성 완료", MAX_WORKERTHREAD);
		return true;
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

			OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped;

			if (IOOperation::ACCEPT == pOverlappedEx->m_operation)
			{
				pSession = m_sessionManager->GetSession(pOverlappedEx->m_sessionIdex);
				if (pSession)
				{
					pSession->AcceptCompleted(m_iocpHandle);
					OnConnect(pSession->GetIndex());
					m_clientCnt.fetch_add(1);
					Session* session = m_sessionManager->GetEmptySession();
					if (session == nullptr)
					{
						Log::Error("초기 세션 부족");
					}
					else if (session->PostAccept(m_listenSocket) == false)
					{
						m_sessionManager->ReturnSession(session);
					}
				}
				continue;
			}

			if (!isSuccess || (dwIoSize == 0 && isSuccess))
			{
				if (pSession)
				{
					CloseSocket(pSession->GetIndex());
				}
				continue;
			}

			if (IOOperation::RECV == pOverlappedEx->m_operation)
			{
				const auto dataSpan{ pSession->GetRecvData(dwIoSize) };
				OnRecv(pSession->GetIndex(), dataSpan);
				pSession->BindRecv();
			}
			else if (IOOperation::SEND == pOverlappedEx->m_operation)
			{
				pSession->SendCompleted(dwIoSize, pOverlappedEx);
				//std::print("[송신] bytes : {:d}\n ", dwIoSize);
			}
			else
			{
				Log::Warning("socket({}에서 예외상황", pSession->GetSocketId());
			}
		}
	}

	void CloseSocket(uint32 index, bool isForce = false)
	{
		m_sessionManager->Close(index, isForce);
		OnClose(index);
		m_sessionManager->ReturnSession(index); 
		m_clientCnt.fetch_sub(1);
	}

	SOCKET m_listenSocket = INVALID_SOCKET;
	std::unique_ptr<SessionManager> m_sessionManager;
	std::atomic<int> m_clientCnt = 0;

	std::vector<std::thread> m_IOWorkerThreads;
	bool m_isWorkerRun = false;


	HANDLE m_iocpHandle = INVALID_HANDLE_VALUE;

};