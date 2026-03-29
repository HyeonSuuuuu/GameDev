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
			if (SendPacket.Num() == 0) break;
			if (Socket)
			{
				int32 BytesSent = 0;
				Socket->Send(SendPacket.GetData(), SendPacket.Num(), BytesSent);
			}
		}
		
		if (Socket)
		{
			uint32 PendingDataSize = 0;
			if (Socket->HasPendingData(PendingDataSize) && PendingDataSize >= sizeof(PacketHeader))
			{
				// 1. 헤더 먼저 읽기
				TArray<uint8> HeaderBuffer;
				HeaderBuffer.SetNumUninitialized(sizeof(PacketHeader));
				int32 BytesRead = 0;
                
				if (Socket->Recv(HeaderBuffer.GetData(), HeaderBuffer.Num(), BytesRead))
				{
					PacketHeader* Header = reinterpret_cast<PacketHeader*>(HeaderBuffer.GetData());
					uint16 FullSize = Header->size;

					// 2. 헤더에 적힌 전체 크기만큼 버퍼 준비
					TArray<uint8> FullPacket;
					FullPacket.SetNumUninitialized(FullSize);
                    
					// 이미 읽은 헤더 부분 복사
					FMemory::Memcpy(FullPacket.GetData(), HeaderBuffer.GetData(), sizeof(PacketHeader));

					// 3. 남은 바디 데이터 읽기
					int32 BodySize = FullSize - sizeof(PacketHeader);
					int32 BodyBytesRead = 0;
                    
					if (BodySize > 0)
					{
						// 실제 데이터가 다 올 때까지 잠시 기다리거나 루프를 돌 수 있지만, 
						// 간단하게 Recv로 남은 부분 보충
						Socket->Recv(FullPacket.GetData() + sizeof(PacketHeader), BodySize, BodyBytesRead);
					}

					// 4. 완성된 패킷을 RecvQueue에 넣기 (서브시스템이 꺼내갈 수 있게)
					EnqueRecvPacket(MoveTemp(FullPacket));
				}
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
