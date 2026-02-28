#include "stdafx.h"
#include "../../../Common/Protocol.h"





int main()
{
	std::print("부팅 시작\n");

	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::print("WSAStartup 실패!\n");
	}

}