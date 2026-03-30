#pragma once
#include "CoreMinimal.h"
#include "Sockets.h"

class FNetworker : public FRunnable
{
public:
	FNetworker();
	virtual ~FNetworker() override;
	
	void EnqueSendPacket(TArray<uint8>&& packet);
	TArray<uint8> DequeSendPacket();
	
	void EnqueRecvPacket(TArray<uint8>&& packet);
	TArray<uint8> DequeRecvPacket();
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
	
	
	void SendLoginPacket(const FString& userID, const FString& userPW);
	
private:
	FSocket* Socket = nullptr;
	bool IsRunning = false;
	TQueue<TArray<uint8>,	EQueueMode::Spsc> SendQueue;
	TQueue<TArray<uint8>,	EQueueMode::Spsc> RecvQueue;
};
