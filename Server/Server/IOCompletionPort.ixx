module;
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

export module IOCompletionPort;

import Common;

namespace IOCompletionPort
{
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
		char m_buf[MAX_SOCKBUF];
		IOOperation m_operation;
	};

	struct ClientInfo
	{
		SOCKET m_socketClient;
		OverlappedEx m_recvOverlappedEx;
		OverlappedEx m_sendOverlappedEx;

		ClientInfo();
	};

	export class IOCompletionPort
	{
	public:
		IOCompletionPort();
		~IOCompletionPort();
		
		bool InitEnvironment();
		bool InitSocket();
		bool BindandListen(const int32 port);
		bool StartServer(const uint32 maxClientCount);
		void DestroyThread();

	private:
		void CreateClient(const uint32 max_ClientCount);
		bool CreateWorkerThread();
		bool CreateAccepterThread();
		ClientInfo* GetEmptyClientInfo();
		bool BindIOCompletionPort(ClientInfo* pClientInfo);
		bool BindRecv(ClientInfo* pClientInfo);
		bool SendMsg(ClientInfo* pClientInfo, char* pMsg, int len);
		void WorkerThread();
		void AccepterThread();
		void CloseSocket(ClientInfo* pClientInfo, bool isForce = false);

		std::vector<ClientInfo> m_clientInfos;
		SOCKET m_listenSocket;
		int m_clientCnt = 0;
		
		std::vector<std::thread> m_IOWorkerThreads;
		std::thread m_accepterThread;
		HANDLE m_IOCPHandle;
		bool m_isWorkerRun;
		bool m_isAccepterRun;
		char m_socketBuf[1024];
	};
}