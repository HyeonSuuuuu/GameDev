// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "OpponentCharacter.h"
#include "NetSubsystem.generated.h"

/**
 * 
 */
class FNetworker;

UCLASS()
class TURTLEISLAND_API UNetSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
public:
	UNetSubsystem() = default;
	virtual ~UNetSubsystem() = default;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	
	void EnqueRecvPacket();
	void DequeRecvPacket();
	
	void EnqueSendPacket(TArray<uint8>&& packet);
	//helper func
	void CopyStringToBuffer(char* Dest, int32 DestSize, const FString& Source);
	
	// 상대방 캐릭터 BP 클래스 끌어다 사용
	UPROPERTY(EditAnywhere, Category = "Network")
	TSubclassOf<class AOpponentCharacter> OpponentClass;
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Network")
	bool IsConnect = false;
	
	FNetworker* Networker = nullptr;
	FRunnableThread* Thread = nullptr;
	
	UPROPERTY()
	TMap<uint32, AOpponentCharacter*> players;
	
	uint32 myId = 0;
};
