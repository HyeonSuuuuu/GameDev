#include "stdafx.h"
#include "../../../Common/Protocol.h"

#include "llamaCpp/llama-cpp.h"
#include "llamaCpp/ggml-backend.h"
#pragma comment(lib, "llamaCpp/llama.lib")
#pragma comment(lib, "llamaCpp/ggml.lib")
#pragma comment(lib, "llamaCpp/ggml-base.lib")
#pragma comment(lib, "llamaCpp/ggml-cpu.lib")


std::string GenerateResponse(
	llama_model* model,
	llama_context* ctx,
	const std::string& userInput);

int main()
{
	// AI 백엔드 초기화
	llama_backend_init();
	ggml_backend_load_all();

	// 모델 파라미터 세팅
	llama_model_params model_params = llama_model_default_params();
	// 몇 개의 신경망 층을 VRAM에 올릴 것인가? (99개 -> 최대한 많이)
	model_params.n_gpu_layers = 99;

	// 모델 파일 경로 설정
	std::string model_path = "models/Llama-3.3-8B-Instruct.Q4_K_M.gguf";
	//std::string model_path = "models/Qwen3.5-9B-Q4_K_M.gguf";
	


	

	Log::Info("GGUF를 VRAM으로 로딩중...");
	Log::Info("Loading .gguf file into vram...");

	// 모델 로딩
	llama_model_ptr model(llama_model_load_from_file(model_path.c_str(), model_params));

	if (!model)
	{
		// 경로 틀리거나 파일이 없다면
		Log::Error("GGUF 모델을 찾을 수 없습니다!");
		Log::Error("Cannot find .gguf model!");
		return -1;
	}
	Log::Info("AI 모델 로딩 완료!");
	Log::Info("successfully loaded AI model!");

	// 컨텍스트 생성 (현재 대화를 기억하는 도화지)
	llama_context_params ctx_params = llama_context_default_params();
	ctx_params.n_ctx = 2048; // 한번에 기억할 수 있는 최대 단어 수
	llama_context_ptr ctx(llama_init_from_model(model.get(), ctx_params));

	if (!ctx)
	{
		Log::Error("AI Context 할당 실패!");
		Log::Error("AI Context allocate failure!");
		return -1;
	}
	Log::Info("AI 준비 완료");
	Log::Info("AI now ready");

	// console test
	while (true)
	{
		printf("테스트 코드입니다. exit를 입력하면 다음 단계로 넘어갑니다.\n");
		std::string input;
		std::print("\nYou > ");
		std::getline(std::cin, input);


		if (input.empty())
			continue;

		if (input == "exit")
			break;

		std::string output = GenerateResponse(model.get(), ctx.get(), input);
		std::print("AI  > {}\n", output);
	}

	//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
	//아래부턴 네트워크 코드입니다.
	//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=


		// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Log::Error("WSAStartup 실패");
		Log::Error("WSAStartup fail");
		return -1;
	}

	// 리슨 소켓 생성
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		Log::Error("소켓 생성 실패");
		Log::Error("socket creation failure");
		WSACleanup();
		return -1;
	}

	// 서버 주소 설정
	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(NetConfig::AI_SERVER_PORT);

	// 바인드
	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		Log::Error("bind 실패");
		Log::Error("bind fail");
		closesocket(listenSocket);
		WSACleanup();
		return -1;
	}

	// 리슨 시작
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		Log::Error("listen 실패");
		Log::Error("listen fail");
		closesocket(listenSocket);
		WSACleanup();
		return -1;
	}

	Log::Info("게임 서버의 연결을 기다리는중... (Port: {})", NetConfig::AI_SERVER_PORT);
	Log::Info("waiting connection with gameServer... (Port: {})", NetConfig::AI_SERVER_PORT);

	// 클라이언트(게임 서버) 접속 대기
	sockaddr_in clientAddr = {};
	int clientAddrSize = sizeof(clientAddr);
	SOCKET gameServerSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);

	if (gameServerSocket == INVALID_SOCKET)
	{
		Log::Error("게임 서버 연결 accept 실패");
		Log::Error("Game server connection accept() failure");
		closesocket(listenSocket);
		WSACleanup();
		return -1;
	}

	Log::Info("게임 서버 접속 완료");
	Log::Info("Successfully accessed game server");

	// 이제 더 이상 listen 소켓은 필요 없음
	closesocket(listenSocket);

	// 문자열 수신/응답 루프
	char recvBuf[2048];

	while (true)
	{
		int recvLen = recv(gameServerSocket, recvBuf, sizeof(recvBuf) - 1, 0);
		if (recvLen <= 0)
		{
			Log::Warning("게임 서버 연결 종료");
			Log::Warning("Game server disconnected");
			break;
		}

		recvBuf[recvLen] = '\0';
		std::string userText = recvBuf;

		// 혹시 줄바꿈이 같이 들어오면 보기 좋게 정리
		while (!userText.empty() &&
			(userText.back() == '\n' || userText.back() == '\r'))
		{
			userText.pop_back();
		}

		if (userText.empty())
			continue;

		Log::Info("받은 문자열: {}", userText);
		Log::Info("Received text: {}", userText);

		// AI 응답 생성
		std::string aiText = GenerateResponse(model.get(), ctx.get(), userText);
		if (aiText.empty())
			aiText = "(No response)";

		// echo + AI 답변을 한 문자열로 합쳐서 한 번만 전송
		std::string sendText;
		sendText += "[echo]\n";
		sendText += userText;
		sendText += "\n\n";
		sendText += "[ai]\n";
		sendText += aiText;
		sendText += "\n<<END>>\n";

		int sendLen = (int)sendText.size();
		int result = send(gameServerSocket, sendText.c_str(), sendLen, 0);
		if (result <= 0)
		{
			Log::Error("응답 전송 실패");
			Log::Error("Failed to send response");
			break;
		}

		Log::Info("응답 전송 완료");
		Log::Info("Response sent");
	}

	closesocket(gameServerSocket);
	WSACleanup();
	return 0;
}


//llama style
std::string GenerateResponse(
	llama_model* model,
	llama_context* ctx,
	const std::string& userInput)
{
	if (model == nullptr || ctx == nullptr || userInput.empty())
		return "";

	const llama_vocab* vocab = llama_model_get_vocab(model);
	if (vocab == nullptr)
	{
		Log::Error("Failed to get model vocab.");
		return "";
	}

	std::string prompt =
		"<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n"
		"You are a helpful assistant. Answer clearly and naturally in Korean.\n"
		"<|eot_id|>\n"
		"<|start_header_id|>user<|end_header_id|>\n" +
		userInput +
		"\n<|eot_id|>\n"
		"<|start_header_id|>assistant<|end_header_id|>\n";

	// 기존 컨텍스트 메모리 초기화
	llama_memory_clear(llama_get_memory(ctx), true);

	// 필요한 토큰 수 계산
	int32_t tokenCount = llama_tokenize(
		vocab,
		prompt.c_str(),
		static_cast<int32_t>(prompt.size()),
		nullptr,
		0,
		false,
		true
	);

	if (tokenCount < 0)
		tokenCount = -tokenCount;

	if (tokenCount <= 0)
	{
		Log::Error("Failed to tokenize prompt.");
		return "";
	}

	// 실제 토큰화
	std::vector<llama_token> tokens(tokenCount);
	int32_t written = llama_tokenize(
		vocab,
		prompt.c_str(),
		static_cast<int32_t>(prompt.size()),
		tokens.data(),
		static_cast<int32_t>(tokens.size()),
		false,
		true
	);

	if (written < 0)
	{
		Log::Error("Prompt tokenization failed.");
		return "";
	}

	// 프롬프트 입력
	llama_batch batch = llama_batch_get_one(tokens.data(), written);
	if (llama_decode(ctx, batch) != 0)
	{
		Log::Error("llama_decode failed on prompt.");
		return "";
	}

	// 샘플러 생성
	llama_sampler_chain_params samplerParams = llama_sampler_chain_default_params();
	llama_sampler* sampler = llama_sampler_chain_init(samplerParams);

	llama_sampler_chain_add(sampler, llama_sampler_init_top_k(40));
	llama_sampler_chain_add(sampler, llama_sampler_init_top_p(0.9f, 1));
	llama_sampler_chain_add(sampler, llama_sampler_init_temp(0.8f));
	llama_sampler_chain_add(sampler, llama_sampler_init_dist(1234));

	std::string response;
	const int maxGenTokens = 256;

	for (int i = 0; i < maxGenTokens; ++i)
	{
		llama_token newToken = llama_sampler_sample(sampler, ctx, -1);

		if (newToken == llama_vocab_eos(vocab) || newToken == 128009)
			break;

		char piece[256] = {};
		int pieceLen = llama_token_to_piece(
			vocab,
			newToken,
			piece,
			static_cast<int32_t>(sizeof(piece)),
			0,
			true
		);

		if (pieceLen > 0)
		{
			response.append(piece, pieceLen);
		}

		llama_batch genBatch = llama_batch_get_one(&newToken, 1);
		if (llama_decode(ctx, genBatch) != 0)
		{
			Log::Error("llama_decode failed during generation.");
			break;
		}
	}

	llama_sampler_free(sampler);
	return response;
}



/*//Qwen3.5 style
std::string GenerateResponse(
	llama_model* model,
	llama_context* ctx,
	const std::string& userInput)
{
	if (model == nullptr || ctx == nullptr)
		return "";

	auto Trim = [](const std::string& s) -> std::string
		{
			size_t start = 0;
			while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
				++start;

			size_t end = s.size();
			while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
				--end;

			return s.substr(start, end - start);
		};

	auto RemoveAllOccurrences = [](std::string& text, const std::string& target)
		{
			size_t pos = 0;
			while ((pos = text.find(target, pos)) != std::string::npos)
			{
				text.erase(pos, target.length());
			}
		};

	auto ContainsAnyStopMarker = [](const std::string& text) -> bool
		{
			static const std::vector<std::string> stopMarkers =
			{
				"<|eot_id|>",
				"<|im_end|>",
				"<|endoftext|>",
				"<|start_header_id|>",
				"<|end_header_id|>",
				"<|assistant|>",
				"<|user|>",
				"\nUser:",
				"\nAssistant:"
			};

			for (const auto& marker : stopMarkers)
			{
				if (text.find(marker) != std::string::npos)
					return true;
			}
			return false;
		};

	auto CutAtFirstStopMarker = [](std::string& text)
		{
			static const std::vector<std::string> stopMarkers =
			{
				"<|eot_id|>",
				"<|im_end|>",
				"<|endoftext|>",
				"<|start_header_id|>",
				"<|end_header_id|>",
				"<|assistant|>",
				"<|user|>",
				"\nUser:",
				"\nAssistant:"
			};

			size_t cutPos = std::string::npos;

			for (const auto& marker : stopMarkers)
			{
				size_t pos = text.find(marker);
				if (pos != std::string::npos)
				{
					if (cutPos == std::string::npos || pos < cutPos)
						cutPos = pos;
				}
			}

			if (cutPos != std::string::npos)
				text.erase(cutPos);
		};

	auto CleanResponseText = [&](std::string text) -> std::string
		{
			RemoveAllOccurrences(text, "<|eot_id|>");
			RemoveAllOccurrences(text, "<|im_end|>");
			RemoveAllOccurrences(text, "<|endoftext|>");
			RemoveAllOccurrences(text, "<|start_header_id|>");
			RemoveAllOccurrences(text, "<|end_header_id|>");
			RemoveAllOccurrences(text, "<|assistant|>");
			RemoveAllOccurrences(text, "<|user|>");

			return Trim(text);
		};

	const std::string cleanInput = Trim(userInput);
	if (cleanInput.empty())
		return "";

	const llama_vocab* vocab = llama_model_get_vocab(model);
	if (vocab == nullptr)
	{
		Log::Error("Failed to get model vocab.");
		return "";
	}

	// Qwen에 Llama3 스타일 헤더 토큰을 직접 넣지 말고,
	// 단순한 텍스트 프롬프트로 구성
	std::string prompt =
		"You are a helpful assistant.\n"
		"Do not output reasoning, hidden thoughts, or chain-of-thought.\n"
		"Return only the final answer.\n\n"
		"User: /no_think " + cleanInput + "\n"
		"Assistant:";

	// 매 요청마다 대화 메모리 초기화
	llama_memory_clear(llama_get_memory(ctx), true);

	int32_t tokenCount = llama_tokenize(
		vocab,
		prompt.c_str(),
		static_cast<int32_t>(prompt.size()),
		nullptr,
		0,
		false,
		true
	);

	if (tokenCount < 0)
		tokenCount = -tokenCount;

	if (tokenCount <= 0)
	{
		Log::Error("Failed to tokenize prompt.");
		return "";
	}

	std::vector<llama_token> tokens(tokenCount);
	int32_t written = llama_tokenize(
		vocab,
		prompt.c_str(),
		static_cast<int32_t>(prompt.size()),
		tokens.data(),
		static_cast<int32_t>(tokens.size()),
		false,
		true
	);

	if (written < 0)
	{
		Log::Error("Prompt tokenization failed.");
		return "";
	}

	llama_batch batch = llama_batch_get_one(tokens.data(), written);
	if (llama_decode(ctx, batch) != 0)
	{
		Log::Error("llama_decode failed on prompt.");
		return "";
	}

	llama_sampler_chain_params samplerParams = llama_sampler_chain_default_params();
	llama_sampler* sampler = llama_sampler_chain_init(samplerParams);

	llama_sampler_chain_add(sampler, llama_sampler_init_top_k(40));
	llama_sampler_chain_add(sampler, llama_sampler_init_top_p(0.9f, 1));
	llama_sampler_chain_add(sampler, llama_sampler_init_temp(0.7f));
	llama_sampler_chain_add(sampler, llama_sampler_init_dist(1234));

	std::string response;
	const int maxGenTokens = 256;

	for (int i = 0; i < maxGenTokens; ++i)
	{
		llama_token newToken = llama_sampler_sample(sampler, ctx, -1);

		if (newToken == llama_vocab_eos(vocab))
			break;

		char piece[256] = {};
		int pieceLen = llama_token_to_piece(
			vocab,
			newToken,
			piece,
			static_cast<int32_t>(sizeof(piece)),
			0,
			true
		);

		if (pieceLen > 0)
		{
			std::string pieceStr(piece, pieceLen);

			if (ContainsAnyStopMarker(pieceStr))
			{
				CutAtFirstStopMarker(pieceStr);
				response += pieceStr;
				break;
			}

			response += pieceStr;

			if (ContainsAnyStopMarker(response))
			{
				CutAtFirstStopMarker(response);
				break;
			}
		}

		llama_batch genBatch = llama_batch_get_one(&newToken, 1);
		if (llama_decode(ctx, genBatch) != 0)
		{
			Log::Error("llama_decode failed during generation.");
			break;
		}
	}

	llama_sampler_free(sampler);

	response = CleanResponseText(response);

	if (response.empty())
		response = "(No response)";

	return response;
}


*/