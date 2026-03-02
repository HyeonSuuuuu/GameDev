export module ErrorCode;

import Common;

export enum class ERROR_CODE : uint16
{
	NONE = 0,

	// User Manager
	USER_MGR_INVALID_INDEX = 11,
	USER_MGR_INVALID_USER_UNIQUE_ID = 12,

	// Login
};