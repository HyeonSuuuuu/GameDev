export module Session;
import Common;
import Define;
import Log;
import <winSock2.h>;
import <ws2tcpip.h>;

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
		return m_socket != INVALID_SOCKET;
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
		bool ret = BindIOCompletionPort(iocpHandle);
		if (ret == false)
		{
			Log::Error("IOCP 바인딩 실패 (Index: {})", m_index);
			return false;
		}

		ret = BindRecv();
		if (ret == false)
		{
			Log::Error("초기 Recv 등록 실패 (Index: {})", m_index);
			return false;
		}

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

		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	bool BindRecv()
	{
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

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

		std::lock_guard<std::mutex> lock(m_sendLock);
		if (msg.size() + m_sendPos > MAX_SOCKBUF)
		{
			m_sendPos = 0;
		}

		char* sendBuf = m_sendBuf + m_sendPos;
		CopyMemory(sendBuf, msg.data(), msg.size());
		m_sendPos += msg.size();
		return true;
	}

	bool SendIO()
	{
		std::lock_guard<std::mutex> lock(m_sendLock);
		if (m_sendPos <= 0 || m_isSending)
		{
			return true;
		}
		m_isSending = true;

		CopyMemory(m_sendingBuf, m_sendBuf, m_sendPos);

		m_sendOverlappedEx.m_wsaBuf.len = m_sendPos;
		m_sendOverlappedEx.m_wsaBuf.buf = m_sendingBuf;
		m_sendOverlappedEx.m_operation = IOOperation::SEND;


		DWORD dwRecvNumBytes = 0;
		int ret = WSASend(m_socket,
			&(m_sendOverlappedEx.m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			reinterpret_cast<LPWSAOVERLAPPED>(&m_sendOverlappedEx),
			nullptr);

		if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			Log::Error("WSASend() 실패 : {}", WSAGetLastError());
			return false;
		}
		m_sendPos = 0;
		return true;
	}

	void SendCompleted(const uint32 dataSize)
	{
		m_isSending = false;
		//Log::Info("[송신 완료] bytes : {}", dataSize);
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
	SOCKET m_socket = INVALID_SOCKET;
	OverlappedEx m_recvOverlappedEx{};
	OverlappedEx m_sendOverlappedEx{};

	char m_recvBuf[MAX_SOCKBUF]{};

	// 1 Send 방식
	std::mutex m_sendLock;
	bool m_isSending = false;
	uint32 m_sendPos = 0;
	char m_sendBuf[MAX_SOCKBUF]{};
	char m_sendingBuf[MAX_SOCKBUF]{};
};