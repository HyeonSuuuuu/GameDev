export module SessionManager;

import Common;
import Define;
import Session;

// LockFree로 수정시 Object Pool + lockfree unordered_map 구조 생각중

export class SessionManager
{
public:
	SessionManager() = delete;
	~SessionManager() = default;

	explicit SessionManager(uint32 maxCount)
	{
		m_sessions.reserve(maxCount);
		for (uint32 i = 0; i < maxCount; ++i)
		{
			m_sessions.emplace_back(std::make_unique<Session>(i));
			m_queue.emplace(0, i);
		}
	}

	

	std::optional<uint32> Pop()
	{
		std::lock_guard<std::mutex> lock(m_lock);
		if (m_queue.empty())
		{
			return std::nullopt;
		}

		auto [closedTime, index] = m_queue.front();
		if (GetTick() - closedTime < REUSE_WAIT_MS)
		{
			return std::nullopt;
		}
		
		m_queue.pop();
		return index;
	}

	void Push(uint32 index)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		m_queue.emplace(GetTick(), index);
		
	}

	void Close(const uint32 index, bool isForce)
	{
		m_sessions[index]->Close(isForce);
	}
	
	bool SendPacket(const uint32 index, const std::span<const char> msg)
	{
		return m_sessions[index]->SendMsg(msg);
	}

	// 수정예정
	Session* GetEmptySession()
	{
		std::optional<uint32> index = Pop();
		if (index.has_value())
		{
			return m_sessions[index.value()].get();
		}
		return nullptr;
	}

	void ReturnSession(Session* session)
	{
		Push(session->GetIndex());
	}

	void ReturnSession(const uint32 sessionIndex)
	{
		Push(sessionIndex);
	}



	Session* GetSession(const uint32 sessionIndex)
	{
		if (sessionIndex >= m_sessions.size())
		{
			return nullptr;
		}
		return m_sessions[sessionIndex].get();
	}

private:

	int64 GetTick()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	std::vector<std::unique_ptr<Session>> m_sessions;
	std::queue<std::pair<int64, uint32>> m_queue;
	std::mutex m_lock;

};