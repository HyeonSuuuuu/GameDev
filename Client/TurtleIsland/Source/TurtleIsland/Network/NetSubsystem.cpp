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

void UNetSubsystem::TryLogin(FString UserID, FString UserPW)
{
	if (Networker)
	{
		Networker->SendLoginPacket(UserID, UserPW);
	}
}
