// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/MyGameInstance.h"
#include "Misc/ScopeLock.h"
void UMyGameInstance::UpdateUserIdSafe(const FString& NewId)
{
	FScopeLock Lock(&mutex);
	user_id = NewId;
}

FString UMyGameInstance::GetUserIdSafe()
{
	FScopeLock Lock(&mutex);
	return user_id;
}
