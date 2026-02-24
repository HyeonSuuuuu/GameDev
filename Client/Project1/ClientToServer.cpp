#include <iostream>
#include <vector>
#include <thread>

#include "Common.h"
#include "Packet.h"

#define SERVERPORT 9000
#define BUFSIZE    512

using namespace std;

// 수신 전용 쓰레드 함수
void ReceiveThread(SOCKET sock) {
    while (true) {
        // 1. 헤더 수신
        PacketHeader header;
        int received = 0;
        int targetSize = PACKET_HEADER_SIZE;

        // TCP는 데이터가 쪼개질 수 있으므로 헤더가 다 올 때까지 반복해서 recv
        while (received < targetSize) {
            int n = recv(sock, reinterpret_cast<char*>(&header) + received, targetSize - received, 0); // recv(sock, 현재 채워야 할 위치의 주소, 남은 크기, 0)

            if (n <= 0) { // 서버가 연결을 끊거나(0) 에러가 발생(-1)하면 쓰레드 종료
                cout << "[수신] 서버 연결 종료" << endl;
                return;
            }
            received += n; // 실제로 받은 바이트 수만큼 누적
        }

        // 2. ID에 따른 패킷 처리 (SC_MOVE_NOTIFY 처리)
        if (header.id == static_cast<uint16>(PACKET_ID::SC_MOVE_NOTIFY)) {
            SC_MoveNotify notify;
            // 헤더 정보 복사
            memcpy(&notify, &header, PACKET_HEADER_SIZE);

            // 나머지 데이터 수신
            int bodySize = header.size - PACKET_HEADER_SIZE;
            int bodyReceived = 0;
            char* bodyPtr = reinterpret_cast<char*>(&notify) + PACKET_HEADER_SIZE;

            while (bodyReceived < bodySize) {
                int n = recv(sock, bodyPtr + bodyReceived, bodySize - bodyReceived, 0);
                if (n <= 0) return;
                bodyReceived += n;
            }

            // 결과 출력
            cout << "\n[Recv Notify] UserIndex: " << notify.userIndex
                << " / Pos: (" << notify.x << ", " << notify.y << ", " << notify.z << ")" << endl;
        }
        else {
            // 다른 패킷 ID가 올 경우: 헤더에 적힌 size만큼 무시하고 넘어가기
            int remainSize = header.size - PACKET_HEADER_SIZE;
            vector<char> dummy(remainSize);
            recv(sock, dummy.data(), remainSize, 0);
        }
    }
}


int main()
{
	// 윈속 초기화 및 서버 연결
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	// TCP 소켓 생성
    SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 서버 주소 설정
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	// 서버에 연결
    if (connect(ClientSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "서버 연결 실패" << endl;
        return 1;
    }
    cout << "서버 연결 성공" << endl;

    //수신 쓰레드 생성 및 시작
    thread t_recv(ReceiveThread, ClientSock);

    // 송신은 메인 쓰레드에서
    while (true) {
        CS_Move movePkt;

        // 헤더 설정
        movePkt.size = sizeof(CS_Move);
        movePkt.id = static_cast<uint16>(PACKET_ID::CS_MOVE);
        movePkt.type = 0; // 속성값 일단 0으로 설정

        // 데이터 설정
        movePkt.inputFlag = 1;
        movePkt.x = 0.0f;
        movePkt.y = 0.0f;
        movePkt.z = 0.0f;
        movePkt.yaw = 45.0f;

        // 전송
        int sent = send(ClientSock, reinterpret_cast<const char*>(&movePkt), sizeof(CS_Move), 0);
        if (sent == SOCKET_ERROR) break;

        cout << "[Send Move] x: " << movePkt.x << " 전송 완료" << endl;
        cout << "[Send Move] y: " << movePkt.y << " 전송 완료" << endl;
        cout << "[Send Move] z: " << movePkt.z << " 전송 완료" << endl;
        cout << "[Send Move] yaw: " << movePkt.yaw << " 전송 완료" << endl;

        this_thread::sleep_for(chrono::milliseconds(17));
    }

	t_recv.join(); //수신 쓰레드가 끝날 때까지 메인이 종료되지 않도록 대기

    closesocket(ClientSock);
    WSACleanup();
    return 0;
}