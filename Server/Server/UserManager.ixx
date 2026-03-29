export module UserManager;

import Common;
import User;
import "../../Common/Protocol.h";

export class UserManager
{
public:
	UserManager() = delete;
	~UserManager() = default;

	UserManager(const uint32 maxUserCnt)
		: m_maxUserCnt{ maxUserCnt }
	{
		m_users.reserve(maxUserCnt);

		for (uint32 i = 0; i < m_maxUserCnt; ++i)
		{
			auto user = std::make_unique<User>(i);
			m_users.emplace_back(std::move(user));
		}
	}

	uint32 GetCurrentuserCnt() const
	{
		return m_currentUserCnt;
	}

	constexpr uint32 GetMaxUserCnt() const
	{
		return m_maxUserCnt;
	}

	void IncreaseUserCnt()
	{
		m_currentUserCnt++;
	}

	void DecreaseUserCnt()
	{
		m_currentUserCnt--;
	}

	void AddUser(uint32 sessionIndex)
	{
		m_users[sessionIndex]->Init();

		return;
	}
	void RemoveUser(uint32 sessionIndex)
	{
		auto pUser = m_users[sessionIndex].get();

		if (pUser->GetDomainState() == User::DOMAIN_STATE::NONE)
			return;

		m_userIdToIndex.erase(pUser->GetUserId());

		m_users[sessionIndex]->Clear();

	}

	ERROR_CODE LoginUser(std::string_view userId, uint32 sessionIndex)
	{
		// 중복 체크

		uint32 userIndex = sessionIndex;
		m_users[userIndex]->SetLogin(userId);
		m_userIdToIndex.emplace(userId, userIndex);

		return ERROR_CODE::NONE;
	}


	std::optional<uint32> FindUserIndexById(const std::string& userId)
	{
		auto ret = m_userIdToIndex.find(userId);
		if (ret == m_userIdToIndex.end())
			return std::nullopt;
		return ret->second;
	}
	
	User* GetUser(uint32 index)
	{
		return 	m_users[index].get();
	}
	
private:
	uint32 m_maxUserCnt = 0;
	uint32 m_currentUserCnt = 0;

	std::vector<std::unique_ptr<User>> m_users;
	std::unordered_map<std::string, uint32> m_userIdToIndex;
};