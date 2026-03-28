// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NetSubsystem.generated.h"

/**
 * 
 */
class FNetworker;

UCLASS()
class TURTLEISLAND_API UNetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY();
public:
	UNetSubsystem() = default;
	virtual ~UNetSubsystem() = default;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category = "Network");
	void TryLogin(FString UserID, FString UserPW);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Network");
	bool IsConnect = false;
	
	FNetworker* Networker = nullptr;
	FRunnableThread* Thread = nullptr;
};
