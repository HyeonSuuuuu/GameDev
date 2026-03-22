#pragma once
#include "CoreMinimal.h"
#include "Sockets.h"

class FNetworker : public FRunnable
{
public:
	FNetworker();
	virtual ~FNetworker() override;
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
	
	void SendLoginPacket(const FString& userID, const FString& userPW);
	
private:
	FSocket* Socket = nullptr;
	bool IsRunning = false;
	
};
