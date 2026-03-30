// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/NetSubsystem.h"

#include <Protocol.h>

#include "FNetworker.h"
#include "Kismet/GameplayStatics.h"

void UNetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	Networker = new FNetworker();
	Thread = FRunnableThread::Create(Networker, TEXT("Network Thread"));
	
	UE_LOG(LogTemp, Warning, TEXT("Network Thread Started!"));
}

void UNetSubsystem::Deinitialize()
{
	if (Networker)
	{
		Networker->Stop();
	}
	if (Thread)
	{
		Thread->Kill(true);
		
		delete Thread;
		Thread = nullptr;
	}
	
	if (Networker)
	{
		delete Networker;
		Networker = nullptr;
	}
	Super::Deinitialize();
}

void UNetSubsystem::Tick(float DeltaTime)
{
	if (Networker == nullptr) return;
	
	TArray<uint8> RecvData = Networker->DequeRecvPacket();
	while (RecvData.Num() > 0)
	{
		if (RecvData.Num() < PACKET_HEADER_SIZE) continue;
		PacketHeader* Header = reinterpret_cast<PacketHeader*>(RecvData.GetData());
		PACKET_ID Id = static_cast<PACKET_ID>(Header->id);
		
		switch (Id)
		{
		case PACKET_ID::SC_LOGIN_RESPONSE:
			{
				SC_Login* Pkt = reinterpret_cast<SC_Login*>(RecvData.GetData());
				
				if (Pkt->result == static_cast<uint8>(ERROR_CODE::NONE))
				{
					// 로그인 성공
					UE_LOG(LogTemp, Log, TEXT("로그인 성공"));
					UGameplayStatics::OpenLevel(GetWorld(), FName("Lv1_Lobby"));

					break;
				}
			}
		}
		
		RecvData = Networker->DequeRecvPacket();
	}

}

TStatId UNetSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UNetSubsystem, STATGROUP_Tickables);
}

void UNetSubsystem::EnqueSendPacket(TArray<uint8>&& packet)
{
	Networker->EnqueSendPacket(MoveTemp(packet));
}

void UNetSubsystem::CopyStringToBuffer(char* Dest, int32 DestSize, const FString& Source)
{
	FTCHARToUTF8 Converter(*Source);
	int32 CopySize = FMath::Min(Converter.Length(), DestSize - 1);
	FMemory::Memcpy(Dest, Converter.Get(), CopySize);
	Dest[CopySize] = '\0'; // 안전하게 마지막 널 문자 삽입
}
