// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OpponentCharacter.generated.h"

USTRUCT(BlueprintType)
struct FOpponentPlayerData
{
	GENERATED_BODY()

	UPROPERTY()
	double X;

	UPROPERTY()
	double Y;

	UPROPERTY()
	double Z;

	UPROPERTY()
	int32 KeyInput;

	// 기본 생성자
	FOpponentPlayerData() : X(0.0), Y(0.0), Z(0.0), KeyInput(0) {}
    
	// 편의를 위한 생성자
	FOpponentPlayerData(double InX, double InY, double InZ, int32 InInput)
		: X(InX), Y(InY), Z(InZ), KeyInput(InInput) {}
};

UCLASS()
class TURTLEISLAND_API AOpponentCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AOpponentCharacter();

	// 고유 ID
	uint32 OpponentPlayerId;

	// 2. 모든 상대방의 데이터를 관리하는 TMap (정적 멤버 혹은 서브시스템 권장)
	// Key: PlayerID (uint32), Value: PlayerData (struct)
	UPROPERTY()
	TMap<uint32, FOpponentPlayerData> OpponentDataMap;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// 데이터를 업데이트하는 함수 예시
	// void UpdateRemotePlayerData(uint32 InId, double InX, double InY, double InZ, int32 InInput);
};