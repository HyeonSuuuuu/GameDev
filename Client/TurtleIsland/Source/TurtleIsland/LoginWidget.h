// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "LoginWidget.generated.h"

/**
 * 
 */
UCLASS()
class TURTLEISLAND_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* LoginButton;
	
	UPROPERTY(meta = (BindWidget))
	class UEditableText* UserIDInput;

	UPROPERTY(meta = (BindWidget))
	class UEditableText* UserPWInput;
	
	
	UFUNCTION()
	void OnLoginButtonClicked();
};
