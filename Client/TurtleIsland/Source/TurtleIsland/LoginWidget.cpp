// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginWidget.h"
#include "Network/NetSubsystem.h"
#include "Protocol.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// 블루프린트의 버튼과 C++ 포인터가 잘 연결되었다면 클릭 이벤트 등록!
	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &ULoginWidget::OnLoginButtonClicked);
	}
}

void ULoginWidget::OnLoginButtonClicked()
{
	// 여기서 서브시스템의 큐에 데이터를 바로 던집니다.
	if (UNetSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetSubsystem>())
	{
		CS_Login LoginPacket {};
		LoginPacket.size = CS_LOGIN_PACKET_SIZE;
		LoginPacket.id = (uint16)PACKET_ID::CS_LOGIN_REQUEST;
		// 버퍼에 담기
		NetSubsystem->CopyStringToBuffer(LoginPacket.userID, MAX_USER_ID_LEN + 1, UserIDInput->GetText().ToString());
		NetSubsystem->CopyStringToBuffer(LoginPacket.userPW, MAX_USER_PW_LEN + 1, UserPWInput->GetText().ToString());
		
		TArray<uint8> SendBuffer;
		SendBuffer.SetNumUninitialized(sizeof(CS_Login));
		FMemory::Memcpy(SendBuffer.GetData(), &LoginPacket, sizeof(CS_Login));
		
		NetSubsystem->EnqueSendPacket(MoveTemp(SendBuffer));
		UE_LOG(LogTemp, Log, TEXT("C++에서 버튼 클릭을 감지하여 큐에 넣었습니다!"));
	}
}
