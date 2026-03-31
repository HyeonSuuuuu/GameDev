// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/NetSubsystem.h"
#include "NetSettings.h"

#include <Protocol.h>

#include "FNetworker.h"
#include "Kismet/GameplayStatics.h"

void UNetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	Networker = new FNetworker();
	Thread = FRunnableThread::Create(Networker, TEXT("Network Thread"));
	
	OpponentClass = UNetSettings::Get()->OpponentClass;

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
					myId = Pkt->playerId;
					// UGameplayStatics::OpenLevel(GetWorld(), FName("Lv1_Lobby"));
					UGameplayStatics::OpenLevel(GetWorld(), FName("Lv3_Ingame")); // 테스트용
				}
			break;
			}
		case PACKET_ID::SC_MOVE_NOTIFY:
			{
				SC_MoveNotify* Pkt = reinterpret_cast<SC_MoveNotify*>(RecvData.GetData());
				if (myId == Pkt->userIndex)
				{
					// 내 캐릭터 이동처리
				}
				else
				{
					// 상대방 캐릭터 이동처리
					if (players.Contains(Pkt->userIndex))
					{
						AOpponentCharacter* Opponent = players[Pkt->userIndex];
						if (Opponent)
						{
							FVector NewLocation(Pkt->x, Pkt->y, Pkt->z);
							FRotator NewRotation(0.f, Pkt->yaw, 0.f);
							Opponent->SetActorLocationAndRotation(NewLocation, NewRotation);
						}
					}
					else
					{
						FVector TestPos(Pkt->x, Pkt->y, Pkt->z); // 초기 위치
						FActorSpawnParameters Params;
						if (OpponentClass)
						{
							AOpponentCharacter* TestActor = GetWorld()->SpawnActor<AOpponentCharacter>(OpponentClass, TestPos, FRotator::ZeroRotator, Params);
							players.Emplace(Pkt->userIndex, TestActor);
							UE_LOG(LogTemp, Log, TEXT("새로운 상대방 스폰"));

						}
					}
				}


				UE_LOG(LogTemp, Log, TEXT("이동 패킷 수신: PlayerID=%d, InputFlag=%d"), Pkt->id, Pkt->inputFlag);
			break;
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
