// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TURTLEISLAND_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GlobalData")
	FString user_id;
	
	FCriticalSection mutex;
	
	UFUNCTION(BlueprintCallable, Category="GlobalData")
	void UpdateUserIdSafe(const FString & NewId);
	UFUNCTION(BlueprintPure, Category="GlobalData")
	FString GetUserIdSafe();
};
