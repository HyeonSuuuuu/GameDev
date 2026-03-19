#include <winSock2.h>
#include <windows.h>
#include <iostream>
#include <chrono>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

#include "../../Common/Protocol.h"

using namespace std;
using namespace chrono;

constexpr int MAX_TEST = 10000;
constexpr int MAX_CLIENTS = MAX_TEST * 2;
constexpr int INVALID_ID = -1;
constexpr int MAX_PACKET_SIZE = 255;
constexpr int MAX_BUFF_SIZE = 255;

HWND g_hWnd;
HANDLE g_hIocp;
std::chrono::high_resolution_clock::time_point last_connect_time;

enum class IOOperation : uint8_t
{
	RECV,
	SEND,
	ACCEPT,
};

struct OverlappedEx : public OVERLAPPED
{
	WSABUF m_wsaBuf;
	unsigned char m_buf[MAX_BUFF_SIZE];
	IOOperation m_operation;
	uint32_t m_sessionIndex = 0;
};

struct Session
{
	int id;
	std::atomic<bool> connected;
	
	SOCKET socket = INVALID_SOCKET;

	// Recv
	OverlappedEx m_recvOverlappedEx{};
	unsigned char m_recvBuf[MAX_PACKET_SIZE]{};
	int prev_packet_data;
	int curr_packet_size;
};

void InitializeNetwork()
{
	//for (auto& cl : g_clients) {

	//}
}
