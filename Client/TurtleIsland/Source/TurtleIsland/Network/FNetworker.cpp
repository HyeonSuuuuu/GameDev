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

void FNetworker::EnqueSendPacket(TArray<uint8>&& packet)
{
	SendQueue.Enqueue((MoveTemp(packet)));
}

TArray<uint8> FNetworker::DequeSendPacket()
{
	TArray<uint8> PopData;
	SendQueue.Dequeue(PopData);
	return PopData;
}

void FNetworker::EnqueRecvPacket(TArray<uint8>&& packet)
{
	RecvQueue.Enqueue(MoveTemp(packet));
}

TArray<uint8> FNetworker::DequeRecvPacket()
{
	TArray<uint8> PopData;
	RecvQueue.Dequeue(PopData);
	return PopData;
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
	while (IsRunning)
	{
		while (true)
		{
			TArray<uint8> SendPacket = DequeSendPacket();
			if (SendPacket.Num() == 0)
			{
				break;
			}
			if (Socket)
			{
				int32 BytesSent = 0;
				Socket->Send(SendPacket.GetData(), SendPacket.Num(), BytesSent);
			}
		}
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
