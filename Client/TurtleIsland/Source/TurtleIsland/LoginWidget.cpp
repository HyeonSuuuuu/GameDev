// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginWidget.h"
#include "Network/NetSubsystem.h"
#include "Protocol.h"
#include "Kismet/GameplayStatics.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();


	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &ULoginWidget::OnLoginButtonClicked);
	}
}

void ULoginWidget::OnLoginButtonClicked()
{
	// 입력된 텍스트 가져오기
	FString ID = UserIDInput ? UserIDInput->GetText().ToString() : TEXT("");
	FString PW = UserPWInput ? UserPWInput->GetText().ToString() : TEXT("");

	// 유효성 검사: 비어있거나 공백만 있는지 확인
	if (ID.TrimStartAndEnd().IsEmpty() || PW.TrimStartAndEnd().IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ID 또는 PW를 입력해주세요."));

		// 사용자에게 알림을 주기 위해 UI에 경고 문구를 띄우는 로직을 여기에 추가할 수 있습니다.

		return; // 입력이 잘못되었으므로 아래 로직(패킷 전송 및 레벨 이동)을 실행하지 않고 종료
	}
	UNetSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetSubsystem>();
	if (NetSubsystem)
	{
		CS_Login LoginPacket {};
		LoginPacket.size = CS_LOGIN_PACKET_SIZE;
		LoginPacket.id = (uint16)PACKET_ID::CS_LOGIN_REQUEST;
		// 버퍼에 담기
		NetSubsystem->CopyStringToBuffer(LoginPacket.userID, MAX_USER_ID_LEN + 1, ID);
		NetSubsystem->CopyStringToBuffer(LoginPacket.userPW, MAX_USER_PW_LEN + 1, PW);
		
		TArray<uint8> SendBuffer;
		SendBuffer.SetNumUninitialized(sizeof(CS_Login));
		FMemory::Memcpy(SendBuffer.GetData(), &LoginPacket, sizeof(CS_Login));
		
		NetSubsystem->EnqueSendPacket(MoveTemp(SendBuffer));
		UE_LOG(LogTemp, Log, TEXT("C++에서 버튼 클릭을 감지하여 큐에 넣었습니다!"));
	}
}