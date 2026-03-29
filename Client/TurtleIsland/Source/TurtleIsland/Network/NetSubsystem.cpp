// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/NetSubsystem.h"
#include "FNetworker.h"

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
