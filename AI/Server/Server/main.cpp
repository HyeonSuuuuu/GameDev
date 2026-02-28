#include "stdafx.h"
#include "../../../Common/Protocol.h"

#include "llamaCpp/llama-cpp.h"
#include "llamaCpp/ggml-backend.h"
#pragma comment(lib, "llamaCpp/llama.lib")
#pragma comment(lib, "llamaCpp/ggml.lib")
#pragma comment(lib, "llamaCpp/ggml-base.lib")
#pragma comment(lib, "llamaCpp/ggml-cpu.lib")
int main()
{
	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Log::Error("WSAStartup 실패");
		return -1;
	}

	// AI 백엔드 초기화
	llama_backend_init();
	ggml_backend_load_all();

	// 모델 파라미터 세팅
	llama_model_params model_params = llama_model_default_params();
	// 몇 개의 신경망 층을 VRAM에 올릴 것인가? (99개 -> 최대한 많이)
	model_params.n_gpu_layers = 99;

	// 모델 파일 경로 설정
	std::string model_path = "models/Llama-3.3-8B-Instruct.Q4_K_M.gguf";
	Log::Info("GGUF를 VRAM으로 로딩중...");
	
	// 모델 로딩
	llama_model_ptr model(llama_model_load_from_file(model_path.c_str(), model_params));

	if (!model)
	{
		// 경로 틀리거나 파일이 없다면
		Log::Error("GGUF 모델을 찾을 수 없습니다!");
		return -1;
	}
	Log::Info("AI 모델 로딩 완료!");
	
	// 컨텍스트 생성 (현재 대화를 기억하는 도화지)
	llama_context_params ctx_params = llama_context_default_params();
	ctx_params.n_ctx = 2048; // 한번에 기억할 수 있는 최대 글자 수
	llama_context_ptr ctx(llama_init_from_model(model.get(), ctx_params));

	if (!ctx)
	{
		Log::Error("AI Context 할당 실패!");
		return -1;
	}
	Log::Info("AI 준비 완료");



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
	
	Log::Info("게임 서버의 연결을 기다리는중... (Port: {})", NetConfig::AI_SERVER_PORT);

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