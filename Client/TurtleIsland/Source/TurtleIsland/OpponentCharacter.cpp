// Fill out your copyright notice in the Description page of Project Settings.

#include "OpponentCharacter.h"

AOpponentCharacter::AOpponentCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AOpponentCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AOpponentCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

