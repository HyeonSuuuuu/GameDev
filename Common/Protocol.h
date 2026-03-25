#pragma once
#include "Type.h"
#include <span>




namespace NetConfig
{
	inline constexpr uint16 GAME_SERVER_PORT = 11021;
	inline constexpr uint16 AI_SERVER_PORT = 4444;
}


// Protocol


enum class PACKET_ID : uint16
{
	// SYSTEM (1~99)
	SYS_USER_CONNECT = 11,
	SYS_USER_DISCONNECT = 12,
	SYS_END = 30,

	// DB (100~199)
	DB_END = 199,

	// Client To Server (200~299)
	CS_LOGIN_REQUEST = 201,

	CS_LOBBY_ENTER_REQUEST = 211,

	CS_ROOM_ENTER_REQUEST = 221,

	CS_ROOM_LEAVE_REQUEST = 231,

	CS_ROOM_CHAT_REQUEST = 241,

	CS_ROOM_START_REQUEST = 252,

	CS_MOVE = 261,

	// Server To Client (300 ~ 399)
	SC_LOGIN_RESPONSE = 301,

	SC_LOBBY_RESPONSE = 311,

	SC_ROOM_ENTER_RESPONSE = 321,
	SC_ROOM_ENTER_NOTIFY = 322,


	SC_ROOM_LEAVE_RESPONSE = 331,

	SC_ROOM_CHAT_RESPONSE = 341,
	SC_ROOM_CHAT_NOTIFY = 342,

	SC_ROOM_START_NOTIFY = 351,

	SC_MOVE_NOTIFY = 361,

	// AI To Server (3000 ~ 3099)
	AS_AI = 3001,

	// Server To AI (3100 ~ 3199)
	SA_AI = 3101,

};

enum class ERROR_CODE : uint8
{
	NONE = 0,

	// User Manager
	USER_LOGIN_USED_ALL = 11,
	/*	USER_MGR_INVALID_INDEX = 11,
		USER_MGR_INVALID_USER_UNIQUE_ID = 12,*/

		// Login
};

#pragma pack(push, 1)
struct PacketHeader
{
	uint16 size;
	uint16 id;
	uint8 type; // 압축여부, 암호화 여부 등 속성을 알아내는 값
};

inline constexpr uint32 PACKET_HEADER_SIZE = sizeof(PacketHeader);


inline constexpr int32 MAX_USER_ID_LEN = 32;
inline constexpr int32 MAX_USER_PW_LEN = 32;
// 로그인 요청
struct CS_Login : public PacketHeader
{
	char userID[MAX_USER_ID_LEN + 1];
	char userPW[MAX_USER_PW_LEN + 1];
};
inline constexpr uint32 CS_LOGIN_PACKET_SIZE = sizeof(CS_Login);
// 로그인 응답
struct SC_Login : public PacketHeader
{
	uint8 result;
};

struct CS_LobbyEnter : public PacketHeader
{

};

struct SC_LobbyEnter : public PacketHeader
{
	// 방 목록
	// 방제목
};

// 룸 입장 요청
struct CS_RoomEnter : public PacketHeader
{
	uint32 roomIndex;
};

struct SC_RoomEnter : public PacketHeader
{
	uint8 result;
	uint32 myUserIndex;
	// PlayerInfo 수
	// 현재 방 유저 정보를 리스트 형태로 보냄 (가변)
};

struct SC_RoomEnterNotify : public PacketHeader
{
	char userID[MAX_USER_ID_LEN + 1];
	uint32 userIndex;
	float x, y, z; // 초기 위치 정보
};

// 룸 나가기 요청
struct CS_RoomLeave : public PacketHeader
{
};

struct SC_RoomLeave : public PacketHeader
{
	uint8 result;
};

struct SC_RoomLeaveNotify : public PacketHeader
{
	uint32 userIndex;
};

// 룸 채팅
inline constexpr int32 MAX_CHAT_MSG_LEN = 256;
struct CS_RoomChat : public PacketHeader
{
	char chatMsg[MAX_CHAT_MSG_LEN + 1];
};

struct SC_RoomChat : public PacketHeader
{
	uint8 result;
};

struct SC_RoomChatNotify : public PacketHeader
{
	uint32 userIndex;
	char chatMsg[MAX_CHAT_MSG_LEN + 1];
};

struct CS_RoomStart : public PacketHeader
{	
};

struct SC_RoomStart : public PacketHeader
{
};

struct CS_Move : public PacketHeader
{
	uint16 inputFlag;
	float yaw;
};

struct SC_MoveNotify : public PacketHeader
{
	uint32 userIndex;
	uint16 inputFlag;
	float x, y, z;
	float yaw;
};


struct SA_AIRequest: public PacketHeader
{
	uint32 userIndex;
	uint32 npcIndex;
};

struct AS_AIResponse : public PacketHeader
{
	uint32 userIndex;
	// 가변 AI 텍스트
};
#pragma pack(pop)
