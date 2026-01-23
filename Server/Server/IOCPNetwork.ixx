module;
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

export module IOCPNetwork;
import Common;
import Define;

export class IOCPNetwork
{
public:
	IOCPNetwork() = default;
	virtual ~IOCPNetwork();
		
	bool InitEnvironment();
	bool InitSocket();
	bool BindandListen(const int32 port);
	bool StartServer(const uint32 maxClientCount);
	void DestroyThread();

	virtual void OnConnect(const uint32 clientIndex) = 0;
	virtual void OnClose(const uint32 clientIndex) = 0;
	virtual void OnRecv(const uint32 clientIndex, const std::span<char> data) = 0;


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
	SOCKET m_listenSocket = INVALID_SOCKET;
	int m_clientCnt = 0;
		
	std::vector<std::thread> m_IOWorkerThreads;
	std::thread m_accepterThread;
	HANDLE m_IOCPHandle = INVALID_HANDLE_VALUE;
	bool m_isWorkerRun = true;
	bool m_isAccepterRun = true;
	char m_socketBuf[1024] {};
};