#include "stdafx.h"
#include "../../../Common/Protocol.h"
#include "llamaCpp/llama-cpp.h"
#include "llamaCpp/ggml-backend.h"

#pragma comment(lib, "llamaCpp/llama.lib")
#pragma comment(lib, "llamaCpp/ggml.lib")
#pragma comment(lib, "llamaCpp/ggml-base.lib")
#pragma comment(lib, "llamaCpp/ggml-cpu.lib")
std::string FormatPromptForModel(
    llama_model* model,
    const std::string& systemPrompt,
    const std::string& userInput);

std::string GenerateResponse(
    llama_model* model,
    llama_context* ctx,
    const std::string& formattedPrompt);

int main()
{
	llama_backend_init();
	ggml_backend_load_all();

	struct BackendGuard
	{
		~BackendGuard() { llama_backend_free(); }
	} backendGuard;

	llama_model_params model_params = llama_model_default_params();
	model_params.n_gpu_layers = 99;

	std::string model_path = "models/Llama-3.3-8B-Instruct.Q4_K_M.gguf";
	//std::string model_path = "models/Qwen3.5-35B-A3B-Q4_K_M.gguf";

	Log::Info("GGUF를 로딩중...");
	Log::Info("Loading .gguf file...");

	llama_model_ptr model(llama_model_load_from_file(model_path.c_str(), model_params));
	if (!model)
	{
		Log::Error("GGUF 모델을 찾을 수 없습니다!");
		Log::Error("Cannot find .gguf model!");
		return -1;
	}

	llama_context_params ctx_params = llama_context_default_params();
	ctx_params.n_ctx = 2048;
	llama_context_ptr ctx(llama_init_from_model(model.get(), ctx_params));
	if (!ctx)
	{
		Log::Error("AI Context 할당 실패!");
		Log::Error("AI Context allocate failure!");
		return -1;
	}

	const std::string systemPrompt =
		"You are a helpful assistant. "
		"Answer clearly and naturally in Korean. "
		"Do not reveal hidden reasoning or chain-of-thought. "
		"Return only the final answer.";

	Log::Info("AI 준비 완료");
	Log::Info("AI now ready");

	while (true)
	{
		printf("테스트 코드입니다. exit를 입력하면 종료합니다.\n");

		std::string input;
		std::print("\nYou > ");
		std::getline(std::cin, input);

		if (input.empty())
			continue;
		if (input == "exit")
			break;

		std::string formattedPrompt =
			FormatPromptForModel(model.get(), systemPrompt, input);
		if (formattedPrompt.empty())
		{
			std::print("AI  > (prompt formatting failed)\n");
			continue;
		}

		std::string output =
			GenerateResponse(model.get(), ctx.get(), formattedPrompt);

		if (output.empty())
			output = "(No response)";

		std::print("AI  > {}\n", output);
	}

	return 0;
}


std::string FormatPromptForModel(
	llama_model* model,
	const std::string& systemPrompt,
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

	const std::string cleanInput = Trim(userInput);
	if (cleanInput.empty())
		return "";

	const char* chatTemplate = llama_model_chat_template(model, nullptr);

	// template가 없거나 비어 있으면 fallback
	if (chatTemplate == nullptr || chatTemplate[0] == '\0')
	{
		Log::Warning("Model chat template not found. Falling back to plain text prompt.");

		std::string fallbackPrompt;
		fallbackPrompt += "System: ";
		fallbackPrompt += systemPrompt;
		fallbackPrompt += "\n";
		fallbackPrompt += "User: ";
		fallbackPrompt += cleanInput;
		fallbackPrompt += "\n";
		fallbackPrompt += "Assistant:";

		return fallbackPrompt;
	}

	std::vector<llama_chat_message> messages;
	messages.push_back({ "system", systemPrompt.c_str() });
	messages.push_back({ "user",   cleanInput.c_str() });

	// 처음엔 적당히 시작하고, 부족하면 필요한 길이만큼 키움
	std::vector<char> buffer(1024);

	for (int attempt = 0; attempt < 4; ++attempt)
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

		// snprintf 계열처럼 "필요한 길이"를 돌려주는 경우를 처리
		if (written < (int32_t)buffer.size())
		{
			return std::string(buffer.data(), static_cast<size_t>(written));
		}

		buffer.resize(static_cast<size_t>(written) + 1);
	}

	Log::Error("llama_chat_apply_template failed after repeated resize.");
	return "";
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
			RemoveAllOccurrences(text, "<|im_end|>");
			RemoveAllOccurrences(text, "<|im_start|>");
			RemoveAllOccurrences(text, "<|endoftext|>");
			RemoveAllOccurrences(text, "<|eot_id|>");
			RemoveAllOccurrences(text, "<|assistant|>");
			RemoveAllOccurrences(text, "<|user|>");
			RemoveAllOccurrences(text, "<|system|>");
			RemoveAllOccurrences(text, "<|start_header_id|>");
			RemoveAllOccurrences(text, "<|end_header_id|>");

			return Trim(text);
		};

	// 매 요청마다 stateless하게 동작
	llama_memory_clear(llama_get_memory(ctx), true);

	// 필요한 토큰 수 계산
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

	// 실제 토큰화
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

	if (sampler == nullptr)
	{
		Log::Error("Failed to create sampler.");
		return "";
	}

	llama_sampler_chain_add(sampler, llama_sampler_init_top_k(40));
	llama_sampler_chain_add(sampler, llama_sampler_init_top_p(0.9f, 1));
	llama_sampler_chain_add(sampler, llama_sampler_init_temp(0.7f));
	llama_sampler_chain_add(sampler, llama_sampler_init_dist(1234));

	std::string response;
	response.reserve(512);

	bool generationError = false;
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

	response = CleanResponseText(response);

	if (response.empty())
	{
		if (generationError)
			return "";

		return "(No response)";
	}

	return response;
}