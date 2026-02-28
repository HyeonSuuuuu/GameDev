#include "stdafx.h"
#include "../../../Common/Protocol.h"





int main()
{
	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Log::Error("WSAStartup 실패");
		return -1;
	}

	// AI 초기화
	

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		Log::Error("소켓 생성 실패");
		WSACleanup();
		return -1;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 게임 서버만 받게
	serverAddr.sin_port = htons(NetConfig::AI_SERVER_PORT);
	
	bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	listen(listenSocket, SOMAXCONN);
	
	Log::Info("게임 서버의 연결을 기다리는중... (Port: {}", NetConfig::AI_SERVER_PORT);

	sockaddr_in clientAddr;
	int clientAddrSize = sizeof(clientAddr);
	SOCKET gameServerSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);

	if (gameServerSocket == INVALID_SOCKET)
	{
		Log::Error("게임 서버 연결 accept 실패");
		closesocket(listenSocket);
		WSACleanup();
		return -1;
	}

	Log::Info("게임 서버 접속 완료");
	closesocket(listenSocket);
	
	// AI 작동

}