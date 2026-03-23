#pragma once
#include <print>
#include <chrono>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Type.h"
namespace Log
{
	enum class Level : uint8
	{
		Info,
		Success,
		Warning,
		Error,
		Critical,
	};

	template<typename... Args>
	void Write(Level level, std::format_string<Args...> fmt, Args&&... args)
	{
		auto now = std::chrono::system_clock::now();
		std::string_view prefix;
		FILE* outStream = stdout;
		switch (level)
		{
		case Level::Info:		prefix = "[Info]";		break;
		case Level::Success:	prefix = "[Success]";	break;
		case Level::Warning:	prefix = "[Warning]";	break;
		case Level::Error:
			prefix = "[Error]";
			outStream = stderr;
			break;
		case Level::Critical:
			prefix = "[Critical]";
			outStream = stderr;
			break;
		}

		std::println(outStream, "[{:%T}] {} {}", now, prefix, std::format(fmt, std::forward<Args>(args)...));
	}

	template<typename... Args>
		void Info(std::format_string<Args...> fmt, Args&&... args) {
		Write(Level::Info, fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
		void Success(std::format_string<Args...> fmt, Args&&... args) {
		Write(Level::Success, fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
		void Warning(std::format_string<Args...> fmt, Args&&... args) {
		Write(Level::Warning, fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
		void Error(std::format_string<Args...> fmt, Args&&... args) {
		Write(Level::Error, fmt, std::forward<Args>(args)...);
	}
	void ErrorDisplay(const char* msg, int errNo) {
		char* msgBuf = nullptr;
		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			errNo,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&msgBuf, 0, NULL);
		std::println("Error: {}", msg);
		std::println("시스템({}): {}",errNo, msgBuf);
		LocalFree(msgBuf);

		while (true);
	}
}