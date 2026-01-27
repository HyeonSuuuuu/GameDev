#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")


constexpr int CLIENT_COUNT = 1'0000'0000;
std::vector<std::thread> threads;

std::atomic<int> g_connectedCount = 0;


void ConnectAndEcho(const std::string& ip, int port)
{
    // 2. 소켓 생성
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cout << "소켓 생성 실패: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // 3. 서버 주소 설정
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port); // 서버 포트와 맞추세요
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    // 4. 서버 연결
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        return;
    }

    g_connectedCount.fetch_add(1);
    char buffer[1024];

    std::string msg = "Hello from Dummy Client!";
    while (true) {

        // 5. 데이터 송신
        int sendRet = send(clientSocket, msg.c_str(), (int)msg.length(), 0);
        if (sendRet == SOCKET_ERROR) {
            break;
        }

        // 6. 데이터 수신 (에코)
        int recvRet = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (recvRet <= 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    g_connectedCount.fetch_sub(1);
    // 7. 종료
    closesocket(clientSocket);

}


int main() {
    std::locale::global(std::locale("ko_KR.UTF-8"));
    // 1. 윈속 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup 실패!" << std::endl;
        return 1;
    }
    threads.reserve(CLIENT_COUNT);
    for (int i = 0; i < CLIENT_COUNT; ++i) {
        threads.emplace_back(std::thread(ConnectAndEcho, "127.0.0.1", 11021));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::cout << "\r현재 연결된 클라이언트: " << g_connectedCount.load();
    }
    
    for (auto& t : threads) t.join();


    WSACleanup();
    return 0;
}