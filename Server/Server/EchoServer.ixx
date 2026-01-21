

export module EchoServer;

import IOCPNetwork;
import Common;

export class EchoServer : public IOCPNetwork
{
	virtual void OnConnect(const uint32 clientIndex) override
	{
		std::print("[Onconnect]: Index({})\n", clientIndex);
	}
		
	virtual void OnClose(const uint32 clientIndex) override
	{
		std::print("[OnClose]: Index({})\n", clientIndex);
	}

	virtual void OnRecv(const uint32 clientIndex, const std::span<char> recvData) override
	{
		std::print("[OnRecv]: Index({}), dataSize({})\n", clientIndex, recvData.size());
	}
};
