module;
#define WIN32_LEAN_AND_MEAN
#include <winSock2.h>;
#include <ws2tcpip.h>;
#include <mswsock.h>

#pragma comment(lib, "mswsock.lib")

export module Session;
import Common;
import Define;
import Log;


export class Session
{
public:
	Session() = delete;
	Session(uint32 index)
		: m_index(index)
	{
	}
	~Session() = default;


	constexpr bool isConnected() const
	{
		return m_isConnect == true;
	}

	constexpr uint32 GetIndex() const
	{
		return m_index;
	}

	constexpr std::span<const char> GetRecvData(int32 size) const
	{
		return std::span<const char>(m_recvBuf, size);
	}

	constexpr uint64 GetSocketId() const
	{
		return static_cast<uint64>(m_socket);
	}

	bool OnConnect(HANDLE iocpHandle, SOCKET socket)
	{
		m_socket = socket;
		m_isConnect = true;
		bool ret = BindIOCompletionPort(iocpHandle);
		if (ret == false)
		{
			Log::Error("IOCP 바인딩 실패 (Index: {})", m_index);
			return false;
		}
		
		m_uid++;
		return true;
	}

	void Close(bool isForce = false)
	{
		linger lingerOpt{ 0, 0 };

		if (isForce)
		{
			lingerOpt.l_onoff = 1;
		}


		shutdown(m_socket, SD_BOTH);

		setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (char*)&lingerOpt, sizeof(lingerOpt));

		m_isConnect = false;

		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	bool PostAccept(SOCKET listenSock)
	{
		m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (m_socket == INVALID_SOCKET)
		{
			Log::Error("client WSASocket() 실패 : {}", GetLastError());
			return false;
		}
		DWORD bytes = 0;
		DWORD flags = 0;

		ZeroMemory(&m_acceptOverlappedEx, sizeof(OverlappedEx));

		m_acceptOverlappedEx.m_wsaBuf.len = 0;
		m_acceptOverlappedEx.m_wsaBuf.buf = nullptr;
		m_acceptOverlappedEx.m_operation = IOOperation::ACCEPT;
		m_acceptOverlappedEx.m_sessionIdex = m_index;
		
		if (AcceptEx(listenSock, m_socket, m_acceptBuf, 0, sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16, &bytes, (LPWSAOVERLAPPED) & (m_acceptOverlappedEx)) == false)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				Log::Error("AcceptEx Error : {}", GetLastError());
				return false;
			}
		}

		return true;
	}

	bool AcceptCompleted(HANDLE iocpHandle)
	{
		Log::Info("AcceptCompletion : SessionIndex({})", m_index);
		if (OnConnect(iocpHandle, m_socket) == false)
		{
			return false;
		}

		sockaddr_in clientAddr{};
		int32 addrLen = sizeof(sockaddr_in);
		char clientIP[32] = { 0 };
		inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, 32 - 1);
		Log::Info("클라이언트 접속 : IP({}) SOCKET({})", clientIP, m_socket);
	}

	bool BindRecv()
	{
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		ZeroMemory(&m_recvOverlappedEx, sizeof(OverlappedEx));

		m_recvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
		m_recvOverlappedEx.m_wsaBuf.buf = m_recvBuf;
		m_recvOverlappedEx.m_operation = IOOperation::RECV;

		int ret = WSARecv(m_socket,
			&(m_recvOverlappedEx.m_wsaBuf),
			1,
			&dwRecvNumBytes,
			&dwFlag,
			(LPWSAOVERLAPPED) & (m_recvOverlappedEx),
			nullptr);

		if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			Log::Error("WSARecv() 실패 : {}", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool SendMsg(const std::span<const char> msg)
	{

		OverlappedEx* sendOverlappedEx = new OverlappedEx();
		sendOverlappedEx->m_wsaBuf.len = static_cast<uint32>(msg.size());
		sendOverlappedEx->m_wsaBuf.buf = new char[msg.size()];
		sendOverlappedEx->m_operation = IOOperation::SEND;
		CopyMemory(sendOverlappedEx->m_wsaBuf.buf, msg.data(), msg.size());

		m_sendDataQueue.emplace_back(sendOverlappedEx);
		
		if (m_sendDataQueue.size() == 1)
		{
			SendIO();
		}
		return true;
	}

	bool SendIO()
	{
		std::lock_guard<std::mutex> lock(m_sendLock);

		if (m_sendDataQueue.empty())
		{
			return false;
		}

		OverlappedEx* sendOverlappedEx = m_sendDataQueue.front();

		DWORD dwRecvNumBytes = 0;
		int ret = WSASend(m_socket,
			&(sendOverlappedEx->m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			reinterpret_cast<LPWSAOVERLAPPED>(sendOverlappedEx),
			nullptr);

		// 연결 실패 처리 (클라이언트 끊김)
		if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			Log::Error("WSASend() 실패 : {}", WSAGetLastError());
			return false;
		}
		m_sendDataQueue.pop_front();

		if (m_sendDataQueue.empty() == false)
		{
			SendIO();
		}
		
		return true;
	}

	void SendCompleted(const uint32 dataSize, OverlappedEx* sendOverlappedEx)
	{
		delete[] sendOverlappedEx->m_wsaBuf.buf;
		delete sendOverlappedEx;
		Log::Info("수신된 dataSize : {}", dataSize);
	}

	bool BindIOCompletionPort(HANDLE iocpHandle) const
	{
		auto hIOCP = CreateIoCompletionPort((HANDLE)m_socket,
			iocpHandle,
			(ULONG_PTR)this,
			0);

		if (hIOCP == nullptr || iocpHandle != hIOCP)
		{
			Log::Error("CreateCompletionPort() 실패: {}", GetLastError());
			return false;
		}

		return true;
	}


	

private:
	uint32 m_index = 0;
	uint32 m_uid = 0;

	bool m_isConnect = false;

	SOCKET m_socket = INVALID_SOCKET;
	
	// Accept
	OverlappedEx m_acceptOverlappedEx{};
	char m_acceptBuf[64];

	// Recv
	OverlappedEx m_recvOverlappedEx{};
	char m_recvBuf[MAX_SOCKBUF]{};

	// Send (queue 사용)
	std::mutex m_sendLock;
	std::deque<OverlappedEx*> m_sendDataQueue;
};