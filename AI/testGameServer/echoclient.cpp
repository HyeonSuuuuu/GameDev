#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable : 4996)

#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

#define PORT 4444
#define BUFSIZE 1024

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (sockaddr*)&addr, sizeof(addr));

    char buf[BUFSIZE];

    while (fgets(buf, BUFSIZE, stdin)) {
        send(sock, buf, (int)strlen(buf), 0);

        std::string response;

        while (true) {
            int len = recv(sock, buf, BUFSIZE - 1, 0);
            if (len <= 0) {
                printf("server disconnected\n");
                closesocket(sock);
                WSACleanup();
                return 0;
            }

            buf[len] = '\0';
            response += buf;

            if (response.find("<<END>>") != std::string::npos)
                break;
        }

        size_t endPos = response.find("<<END>>");
        if (endPos != std::string::npos)
            response.erase(endPos);

        printf("%s\n", response.c_str());
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}