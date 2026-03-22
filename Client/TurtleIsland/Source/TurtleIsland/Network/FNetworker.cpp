#include "FNetworker.h"
#include "SocketSubsystem.h"
#include "Protocol.h"
#include "Interfaces/IPv4/IPv4Address.h"

FNetworker::FNetworker()
{
}

FNetworker::~FNetworker()
{
	Exit();
}

bool FNetworker::Init()
{
	IsRunning = true;
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(TEXT("Stream"), TEXT("Client Socket"));
	if (Socket == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Socket Creation Failed"));
		return false;
	}
	
	FString address = TEXT("127.0.0.1");
	int32 port = NetConfig::GAME_SERVER_PORT;
	
	FIPv4Address ip;
	FIPv4Address::Parse(address, ip);
	
	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(port);
	
	
	return Socket->Connect(*addr);
}

uint32 FNetworker::Run()
{
	CS_Login loginPacket {};
	loginPacket.id = static_cast<uint16>(PACKET_ID::CS_LOGIN_REQUEST);
	loginPacket.size = CS_LOGIN_PACKET_SIZE;
	const char* ID = "hyeonsu1234";
	const char* PW = "hyeonsu8900";
	FMemory::Memcpy(loginPacket.userID, ID, MAX_USER_ID_LEN);
	FMemory::Memcpy(loginPacket.userPW, PW, MAX_USER_PW_LEN);
	
	int32 byteSent = 0;
	bool retval = Socket->Send((uint8*)&loginPacket, CS_LOGIN_PACKET_SIZE, byteSent);
	if (retval && byteSent > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Send Success! Sent %d bytes"), byteSent);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Send Failed!"));
	}
	
	while (IsRunning)
	{
		FPlatformProcess::Sleep(0.01f);
	}
	return 0;
}

void FNetworker::Exit()
{
	Stop();
	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
	FRunnable::Exit();
}

void FNetworker::Stop()
{
	IsRunning = false;
	FRunnable::Stop();
}
