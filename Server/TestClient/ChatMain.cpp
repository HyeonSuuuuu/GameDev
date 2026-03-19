#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#include "../../Common/Protocol.h"

int main() {
    // 1. 윈속 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup 실패!" << std::endl;
        return 1;
    }

    // 2. 소켓 생성
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cout << "소켓 생성 실패: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // 3. 서버 주소 설정
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(11021); // 서버 포트와 맞추세요
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);



    while (true) {
        // 4. 서버 연결
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cout << "서버 연결 실패: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "서버에 연결되었습니다! (종료하려면 'quit' 입력)" << std::endl;


        CS_Login loginPacket{};
        loginPacket.size = CS_LOGIN_PACKET_SIZE;
        loginPacket.id = static_cast<uint16>(PACKET_ID::CS_LOGIN_REQUEST);

        strncpy_s(loginPacket.userID, "이현수", MAX_USER_ID_LEN);
        strncpy_s(loginPacket.userPW, "비밀번호", MAX_USER_PW_LEN);

        int sendRet = send(clientSocket, reinterpret_cast<const char*>(&loginPacket), loginPacket.size, 0);
        if (sendRet == SOCKET_ERROR) {
            std::cout << "송신 실패: " << WSAGetLastError() << std::endl;
            break;
        }

        while (true) {

            // 5. 데이터 송신


            



            // 6. 데이터 수신 (에코)
/*            int recvRet = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (recvRet <= 0) {
                std::cout << "서버와 연결이 끊겼습니다." << std::endl;
                break;
            }

            buffer[recvRet] = '\0';
            std::cout << "서버로부터 받은 메시지: " << buffer << std::endl;*/
        }
    }


    // 7. 종료
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}