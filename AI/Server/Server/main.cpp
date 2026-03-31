#include <iostream>
#include <print>
#include <thread>
#include <vector>
#include <winSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "../../../Common/Type.h"
#include "../../../Common/Log.h"
#include "../../../Common/Protocol.h"
#include "llamaCpp/llama-cpp.h"
#include "llamaCpp/ggml-backend.h"

#pragma comment(lib, "llamaCpp/llama.lib")
#pragma comment(lib, "llamaCpp/ggml.lib")
#pragma comment(lib, "llamaCpp/ggml-base.lib")
#pragma comment(lib, "llamaCpp/ggml-cpu.lib")

//==================================debug
#include <filesystem>

//==================================debug





struct ChatTurn
{
	std::string role;
	std::string text;
};

constexpr int kContextTokens = 8192;
constexpr int kMaxGenTokens = 256;
constexpr int kReservedTokens = 512;
constexpr int kPromptBudget = kContextTokens - kMaxGenTokens - kReservedTokens;

std::string FormatPromptForModel(
	llama_model* model,
	const std::string& systemPrompt,
	const std::vector<ChatTurn>& history,
	const std::string& userInput);

std::string GenerateResponse(
	llama_model* model,
	llama_context* ctx,
	const std::string& formattedPrompt);


int main() {
	llama_backend_init();
	ggml_backend_load_all();

	struct BackendGuard
	{
		~BackendGuard() { llama_backend_free(); }
	} backendGuard;

	llama_model_params model_params = llama_model_default_params();
	model_params.n_gpu_layers = 99;

	//place-you-model-here
	std::string model_path = "models/gpt-oss-20b-Q4_K_M.gguf";

	Log::Info("GGUF를 로딩중...");
	Log::Info("Loading .gguf file...");

	//==================================debug

	std::cout << "[Debug] model_path : " << model_path << std::endl;
	std::cout << "[Debug] cwd        : " << std::filesystem::current_path().string() << std::endl;
	std::cout << "[Debug] full path  : "
		<< std::filesystem::absolute(std::filesystem::path(model_path)).string()
		<< std::endl;
	std::cout << "[Debug] exists     : "
		<< (std::filesystem::exists(std::filesystem::path(model_path)) ? "YES" : "NO")
		<< std::endl;

	//==================================debug

	llama_model_ptr model(llama_model_load_from_file(model_path.c_str(), model_params));

	if (!model)
	{
		Log::Error("GGUF 모델을 찾을 수 없습니다!");
		Log::Error("Cannot find .gguf model!");
		return -1;
	}

	llama_context_params ctx_params = llama_context_default_params();
	ctx_params.n_ctx = kContextTokens;
	llama_context_ptr ctx(llama_init_from_model(model.get(), ctx_params));
	if (!ctx)
	{
		Log::Error("AI Context 할당 실패!");
		Log::Error("AI Context allocate failure!");
		return -1;
	}

	const std::string systemPrompt =
		"Reasoning: low. "
		"You are an in-game NPC for roleplay conversation. "
		"Always answer naturally in Korean. "
		"Stay in character and reply like spoken dialogue. "
		"Keep answers concise and direct. "
		"Do not explain your reasoning. "
		"Do not output analysis. "
		"Do not output hidden thoughts. "
		"Do not output tags such as <think>, </think>, <analysis>, </analysis>. "
		"Do not narrate instructions. "
		"Do not add prefixes like 'Final answer:', 'Answer:', '답변:', or speaker labels. "
		"Output only the final spoken reply.";

	std::vector<ChatTurn> history;

	Log::Info("AI 준비 완료");
	Log::Info("AI now ready");
	//AI 작동 확인
	while (true)
	{
		printf("테스트 코드입니다. exit를 입력하면 종료합니다.\n");
		printf("/reset 을 입력하면 대화 기억을 초기화합니다.\n");

		std::string input;
		std::print("\nYou > ");
		std::getline(std::cin, input);

		if (input.empty())
			continue;
		if (input == "exit")
			break;

		if (input == "/reset")
		{
			history.clear();
			llama_memory_clear(llama_get_memory(ctx.get()), true);
			std::print("AI  > 대화 기억을 초기화했습니다.\n");
			continue;
		}

		std::string formattedPrompt =
			FormatPromptForModel(model.get(), systemPrompt, history, input);

		if (formattedPrompt.empty())
		{
			std::print("AI  > (prompt formatting failed)\n");
			continue;
		}

		std::string output =
			GenerateResponse(model.get(), ctx.get(), formattedPrompt);

		const bool validResponse = !output.empty();

		if (!validResponse)
			output = "(No response)";

		std::print("AI  > {}\n", output);

		history.push_back({ "user", input });

		if (validResponse)
		{
			history.push_back({ "assistant", output });
		}
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

		// 줄바꿈 제거
		while (!userText.empty() &&
			(userText.back() == '\n' || userText.back() == '\r'))
		{
			userText.pop_back();
		}

		if (userText.empty())
			continue;

		Log::Info("받은 문자열: {}", userText);
		Log::Info("Received text: {}", userText);

		// 여기서 반드시 프롬프트를 먼저 만든 뒤 AI 호출
		std::string formattedPrompt =
			FormatPromptForModel(model.get(), systemPrompt, history, userText);

		std::string aiText;
		bool validResponse = false;

		if (!formattedPrompt.empty())
		{
			aiText = GenerateResponse(model.get(), ctx.get(), formattedPrompt);
			validResponse = !aiText.empty();
		}

		if (!validResponse)
			aiText = "(No response)";

		// history 반영
		history.push_back({ "user", userText });
		if (validResponse)
		{
			history.push_back({ "assistant", aiText });
		}

		// echo + AI 답변을 한 문자열로 합쳐서 한 번만 전송
		std::string sendText;
		sendText += "[echo]\n";
		sendText += userText;
		sendText += "\n\n";
		sendText += "[ai]\n";
		sendText += aiText;
		sendText += "\n<<END>>\n";

		int totalSent = 0;
		const int sendLen = (int)sendText.size();

		while (totalSent < sendLen)
		{
			int sent = send(
				gameServerSocket,
				sendText.c_str() + totalSent,
				sendLen - totalSent,
				0
			);

			if (sent <= 0)
			{
				Log::Error("응답 전송 실패");
				Log::Error("Failed to send response");
				closesocket(gameServerSocket);
				WSACleanup();
				return -1;
			}

			totalSent += sent;
		}

		Log::Info("응답 전송 완료");
		Log::Info("Response sent");
	}

	closesocket(gameServerSocket);
	WSACleanup();
	

}

std::string FormatPromptForModel(
	llama_model* model,
	const std::string& systemPrompt,
	const std::vector<ChatTurn>& history,
	const std::string& userInput)
{
	if (model == nullptr)
		return "";

	auto Trim = [](const std::string& s) -> std::string
		{
			size_t start = 0;
			while (start < s.size() &&
				std::isspace(static_cast<unsigned char>(s[start])))
			{
				++start;
			}

			size_t end = s.size();
			while (end > start &&
				std::isspace(static_cast<unsigned char>(s[end - 1])))
			{
				--end;
			}

			return s.substr(start, end - start);
		};

	auto BuildPromptFromMessages =
		[&](const std::vector<llama_chat_message>& messages) -> std::string
		{
			const char* chatTemplate = llama_model_chat_template(model, nullptr);

			if (chatTemplate == nullptr || chatTemplate[0] == '\0')
			{
				Log::Warning("Model chat template not found. Falling back to plain text prompt.");

				std::string fallbackPrompt;
				for (const auto& msg : messages)
				{
					if (std::string(msg.role) == "system")
						fallbackPrompt += "System: ";
					else if (std::string(msg.role) == "user")
						fallbackPrompt += "User: ";
					else
						fallbackPrompt += "Assistant: ";

					fallbackPrompt += msg.content;
					fallbackPrompt += "\n";
				}

				fallbackPrompt += "Assistant:";
				return fallbackPrompt;
			}

			std::vector<char> buffer(2048);

			for (int attempt = 0; attempt < 6; ++attempt)
			{
				int32_t written = llama_chat_apply_template(
					chatTemplate,
					messages.data(),
					messages.size(),
					true,
					buffer.data(),
					(int32_t)buffer.size()
				);

				if (written < 0)
				{
					Log::Error("llama_chat_apply_template failed.");
					return "";
				}

				if (written < (int32_t)buffer.size())
				{
					return std::string(buffer.data(), static_cast<size_t>(written));
				}

				buffer.resize(static_cast<size_t>(written) + 1);
			}

			Log::Error("llama_chat_apply_template failed after repeated resize.");
			return "";
		};

	auto CountTokens =
		[&](const std::string& text) -> int32_t
		{
			const llama_vocab* vocab = llama_model_get_vocab(model);
			if (vocab == nullptr)
				return -1;

			int32_t count = llama_tokenize(
				vocab,
				text.c_str(),
				(int32_t)text.size(),
				nullptr,
				0,
				false,
				true
			);

			if (count < 0)
				count = -count;

			return count;
		};

	const std::string cleanInput = Trim(userInput);
	if (cleanInput.empty())
		return "";

	size_t startIndex = 0;

	while (true)
	{
		std::vector<llama_chat_message> messages;
		messages.push_back({ "system", systemPrompt.c_str() });

		for (size_t i = startIndex; i < history.size(); ++i)
		{
			messages.push_back({
				history[i].role.c_str(),
				history[i].text.c_str()
				});
		}

		messages.push_back({ "user", cleanInput.c_str() });

		std::string prompt = BuildPromptFromMessages(messages);
		if (prompt.empty())
			return "";

		int32_t tokenCount = CountTokens(prompt);
		if (tokenCount < 0)
		{
			Log::Error("Failed to count prompt tokens.");
			return "";
		}

		if (tokenCount <= kPromptBudget)
		{
			std::cout << "[Debug] prompt tokens : " << tokenCount
				<< " / budget " << kPromptBudget << std::endl;
			return prompt;
		}

		if (startIndex < history.size())
		{
			++startIndex;
			continue;
		}

		Log::Error("Prompt is too long even without history.");
		return "";
	}
}

std::string GenerateResponse(
	llama_model* model,
	llama_context* ctx,
	const std::string& formattedPrompt)
{
	if (model == nullptr || ctx == nullptr || formattedPrompt.empty())
		return "";

	const llama_vocab* vocab = llama_model_get_vocab(model);
	if (vocab == nullptr)
	{
		Log::Error("Failed to get model vocab.");
		return "";
	}

	auto Trim = [](const std::string& s) -> std::string
		{
			size_t start = 0;
			while (start < s.size() &&
				std::isspace(static_cast<unsigned char>(s[start])))
			{
				++start;
			}

			size_t end = s.size();
			while (end > start &&
				std::isspace(static_cast<unsigned char>(s[end - 1])))
			{
				--end;
			}

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

	auto StripLeadingPrefixes = [&](std::string& text)
		{
			static const std::vector<std::string> prefixes =
			{
				"Final answer:",
				"Final Answer:",
				"Answer:",
				"답변:",
				"최종 답변:",
				"Assistant:",
				"AI:",
				"smith:"
			};

			bool removed = true;
			while (removed)
			{
				removed = false;
				text = Trim(text);

				for (const auto& prefix : prefixes)
				{
					if (text.size() >= prefix.size() &&
						text.compare(0, prefix.size(), prefix) == 0)
					{
						text.erase(0, prefix.size());
						removed = true;
						break;
					}
				}
			}
		};

	auto LooksLikeEnglishMetaStart = [](const std::string& text) -> bool
		{
			static const std::vector<std::string> markers =
			{
				"We need to respond",
				"The user is asking",
				"We should respond",
				"The prompt says",
				"User says",
				"We must respond",
				"We need to answer",
				"The user says",
				"We are asked",
				"The assistant should",
				"We should answer",
				"We need to",
				"The user:"
			};

			for (const auto& marker : markers)
			{
				if (text.size() >= marker.size() &&
					text.compare(0, marker.size(), marker) == 0)
				{
					return true;
				}
			}

			return false;
		};

	auto FindFinalContentStart = [&](const std::string& text) -> size_t
		{
			size_t pos = std::string::npos;

			pos = text.find("<|channel|>final<|message|>");
			if (pos != std::string::npos)
				return pos + std::string("<|channel|>final<|message|>").size();

			pos = text.find("Final answer:");
			if (pos != std::string::npos)
				return pos + std::string("Final answer:").size();

			pos = text.find("Final Answer:");
			if (pos != std::string::npos)
				return pos + std::string("Final Answer:").size();

			pos = text.find("답변:");
			if (pos != std::string::npos)
				return pos + std::string("답변:").size();

			pos = text.find("최종 답변:");
			if (pos != std::string::npos)
				return pos + std::string("최종 답변:").size();

			return std::string::npos;
		};

	auto ContainsAnyStopMarker = [](const std::string& text) -> bool
		{
			static const std::vector<std::string> stopMarkers =
			{
				"<|im_end|>",
				"<|im_start|>",
				"<|endoftext|>",
				"<|eot_id|>",
				"<|assistant|>",
				"<|user|>",
				"<|system|>",
				"<|start_header_id|>",
				"<|end_header_id|>",
				"<think>",
				"</think>",
				"<analysis>",
				"</analysis>",
				"Thinking Process:",
				"Reasoning:",
				"Internal:",
				"<|return|>",
				"\nUser:",
				"\nAssistant:",
				"\nSystem:"
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
				"<|im_end|>",
				"<|im_start|>",
				"<|endoftext|>",
				"<|eot_id|>",
				"<|assistant|>",
				"<|user|>",
				"<|system|>",
				"<|start_header_id|>",
				"<|end_header_id|>",
				"<think>",
				"</think>",
				"<analysis>",
				"</analysis>",
				"Thinking Process:",
				"Reasoning:",
				"Internal:",
				"<|return|>",
				"\nUser:",
				"\nAssistant:",
				"\nSystem:"
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
			size_t finalStart = FindFinalContentStart(text);
			if (finalStart != std::string::npos)
			{
				text = text.substr(finalStart);
			}

			RemoveAllOccurrences(text, "<|start|>");
			RemoveAllOccurrences(text, "<|end|>");
			RemoveAllOccurrences(text, "<|message|>");
			RemoveAllOccurrences(text, "<|channel|>analysis");
			RemoveAllOccurrences(text, "<|channel|>final");
			RemoveAllOccurrences(text, "<|channel|>commentary");
			RemoveAllOccurrences(text, "<|return|>");

			RemoveAllOccurrences(text, "<|im_end|>");
			RemoveAllOccurrences(text, "<|im_start|>");
			RemoveAllOccurrences(text, "<|endoftext|>");
			RemoveAllOccurrences(text, "<|eot_id|>");
			RemoveAllOccurrences(text, "<|assistant|>");
			RemoveAllOccurrences(text, "<|user|>");
			RemoveAllOccurrences(text, "<|system|>");
			RemoveAllOccurrences(text, "<|start_header_id|>");
			RemoveAllOccurrences(text, "<|end_header_id|>");
			RemoveAllOccurrences(text, "<think>");
			RemoveAllOccurrences(text, "</think>");
			RemoveAllOccurrences(text, "<analysis>");
			RemoveAllOccurrences(text, "</analysis>");

			size_t pos = std::string::npos;

			pos = text.find("Thinking Process:");
			if (pos != std::string::npos)
				text.erase(pos);

			pos = text.find("Reasoning:");
			if (pos != std::string::npos)
				text.erase(pos);

			pos = text.find("Internal:");
			if (pos != std::string::npos)
				text.erase(pos);

			pos = text.find("<|channel|>analysis<|message|>");
			if (pos != std::string::npos)
				text.erase(pos);

			pos = text.find("<|end|>");
			if (pos != std::string::npos)
				text.erase(pos);

			pos = text.find("<|start|>");
			if (pos != std::string::npos)
				text.erase(pos);

			StripLeadingPrefixes(text);
			text = Trim(text);

			// 맨 앞이 영문 메타면, 첫 줄/첫 문단을 벗겨보고 다시 본다.
			if (LooksLikeEnglishMetaStart(text))
			{
				size_t nl = text.find('\n');
				if (nl != std::string::npos)
				{
					text = Trim(text.substr(nl + 1));
				}
			}

			if (LooksLikeEnglishMetaStart(text))
			{
				size_t dbl = text.find("\n\n");
				if (dbl != std::string::npos)
				{
					text = Trim(text.substr(dbl + 2));
				}
			}

			StripLeadingPrefixes(text);
			text = Trim(text);

			if (LooksLikeEnglishMetaStart(text))
				return "";

			return text;
		};

	auto RunOnce = [&](uint32_t seed) -> std::string
		{
			llama_memory_clear(llama_get_memory(ctx), true);

			int32_t tokenCount = llama_tokenize(
				vocab,
				formattedPrompt.c_str(),
				(int32_t)formattedPrompt.size(),
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

			std::vector<llama_token> tokens((size_t)tokenCount);
			int32_t written = llama_tokenize(
				vocab,
				formattedPrompt.c_str(),
				(int32_t)formattedPrompt.size(),
				tokens.data(),
				(int32_t)tokens.size(),
				false,
				true
			);

			if (written < 0)
			{
				Log::Error("Prompt tokenization failed.");
				return "";
			}

			if (written == 0)
			{
				Log::Error("Prompt tokenization returned zero tokens.");
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

			if (sampler == nullptr)
			{
				Log::Error("Failed to create sampler.");
				return "";
			}

			llama_sampler_chain_add(sampler, llama_sampler_init_top_k(0));
			llama_sampler_chain_add(sampler, llama_sampler_init_top_p(1.0f, 1));
			llama_sampler_chain_add(sampler, llama_sampler_init_min_p(0.03f, 1));
			llama_sampler_chain_add(sampler, llama_sampler_init_temp(0.90f));
			llama_sampler_chain_add(sampler, llama_sampler_init_dist(seed));

			std::string response;
			response.reserve(512);

			bool generationError = false;

			for (int i = 0; i < kMaxGenTokens; ++i)
			{
				llama_token newToken = llama_sampler_sample(sampler, ctx, -1);

				if (newToken == llama_vocab_eos(vocab))
					break;

				char piece[256] = {};
				int pieceLen = llama_token_to_piece(
					vocab,
					newToken,
					piece,
					(int32_t)sizeof(piece),
					0,
					true
				);

				if (pieceLen < 0)
				{
					Log::Error("llama_token_to_piece failed.");
					generationError = true;
					break;
				}

				if (pieceLen > 0)
				{
					response.append(piece, static_cast<size_t>(pieceLen));

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
					generationError = true;
					break;
				}
			}

			llama_sampler_free(sampler);

			if (generationError)
				return "";

			return CleanResponseText(response);
		};

	std::string response = RunOnce(1234);
	if (!response.empty())
		return response;

	Log::Warning("Response cleanup failed or meta output leaked. Retrying once...");

	response = RunOnce(5678);
	if (!response.empty())
		return response;

	return "";
}