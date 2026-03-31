// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NetSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta = (DisplayName = "Network Settings"))
class TURTLEISLAND_API UNetSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Classes")
    TSubclassOf<class AOpponentCharacter> OpponentClass;

    // 싱글톤처럼 어디서든 설정값을 가져오기 위한 헬퍼 함수
    static const UNetSettings* Get() { return GetDefault<UNetSettings>(); }
};
