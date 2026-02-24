#include <iostream>
#include "..\lib\httplib.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;


int main()
{
	httplib::Client client("localhost", 8080);

	json request_body =
	{
		{"messages", {
			{{"role", "system"}, {"content", "당신은 판타지 게임의 무기 상인입니다. 거칠지만 정이 많습니다"}},
			{{ "role", "user" }, {"content", "제일 좋은 검 하나 보여주세요."} }
	}	},
		{ "temperature", 0.7 },
		{"max_tokens", 150}
	};

	auto ret = client.Post("/v1/chat/completions", request_body.dump(), "application/json");
	
	if (ret && ret->status == 200)
	{
		json response_body = json::parse(ret->body);

	std::string ai_reply = response_body["choices"][0]["message"]["content"];

	std::cout << "무기 상인 NPC: " << ai_reply << std::endl;

	}
	else
	{
		std::cout << "AI 서버 연결 실패 또는 에러 발생" << std::endl;
		if (ret)
		{
			std::cout << "에러 상태 코드: " << ret->status << std::endl;
		}
	}

	return 0;
}



