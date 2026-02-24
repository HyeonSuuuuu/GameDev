#include <iostream>
#include <span>

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

using float32 = float;
using float64 = double;

using byte = unsigned char;


struct Packet
{
	uint32 clientIndex;
	uint16 packetId;
	std::span<const char> dataSpan;
};

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
	CS_ROOM_ENTER_REQUEST = 211,
	CS_ROOM_LEAVE_REQUEST = 221,
	CS_ROOM_CHAT_REQUEST = 231,
	CS_MOVE = 241,

	// Server To Client (300 ~ 399)
	SC_LOGIN_RESPONSE = 301,

	SC_ROOM_ENTER_RESPONSE = 311,
	SC_ROOM_ENTER_NOTIFY = 312,


	SC_ROOM_LEAVE_RESPONSE = 321,

	SC_ROOM_CHAT_RESPONSE = 331,
	SC_ROOM_CHAT_NOTIFY = 332,

	SC_MOVE_NOTIFY = 341,

};

#pragma pack(push, 1)
struct PacketHeader
{
	uint16 size;
	uint16 id;
	uint8 type; // 압축여부, 암호화 여부 등 속성을 알아내는 값
};

constexpr uint32 PACKET_HEADER_SIZE = sizeof(PacketHeader);


constexpr int32 MAX_USER_ID_LEN = 32;
constexpr int32 MAX_USER_PW_LEN = 32;
// 로그인 요청
struct CS_Login : public PacketHeader
{
	char userID[MAX_USER_ID_LEN + 1];
	char userPW[MAX_USER_PW_LEN + 1];
};
constexpr uint32 CS_LOGIN_REQUEST_SIZE = sizeof(CS_Login);
// 로그인 응답
struct SC_Login : public PacketHeader
{
	uint8 result;
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
constexpr int32 MAX_CHAT_MSG_LEN = 256;
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

struct CS_Move : public PacketHeader
{
	uint16 inputFlag;
	float x, y, z;
	float yaw;
};

struct SC_MoveNotify : public PacketHeader
{
	uint32 userIndex;
	uint16 inputFlag;
	float x, y, z;
	float yaw;
};
#pragma pack(pop)