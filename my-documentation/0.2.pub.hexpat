import std.core;
import std.io;
import std.time;
import std.sys;

using BitfieldOrder = std::core::BitfieldOrder;

// Constant values

enum Version : u8 {
    V0 = 0x00, // 0b00000000
    V1 = 0x01, // 0b00000001
    V2 = 0x02, // 0b00000010
};

enum Type : u8 {
    Server_Registration_Request =   0x00, // 0b00000000
    Server_Registration_Response =  0x01, // 0b00000001
    Server_Health_Check_Request =   0x04, // 0b00000100
    Server_Health_Check_Response =  0x05, // 0b00000101
    Server_Activation_Request =     0x08, // 0b00001000
    Server_Activation_Response =    0x09, // 0b00001001
    Get_Activated_Server_Request =  0x0A, // 0b00001010
    Get_Activated_Server_Response = 0x0B, // 0b00001011
    Server_Deactivation_Request =   0x0E, // 0b00001110
    Server_Deactivation_Response =  0x0F, // 0b00001111
    User_Registration_Request =     0x10, // 0b00010000
    User_Registration_Response =    0x11, // 0b00010001
    Get_User_Info_Request =         0x12, // 0b00010010
    Get_User_Info_Response =        0x13, // 0b00010011
    Login_or_Logout_Request =       0x14, // 0b00010100
    Login_or_Logout_Response =      0x15, // 0b00010101
    Log_Request =                   0x18, // 0b00011000
    Log_Response =                  0x19, // 0b00011001
    Get_Channel_Info_Request =      0x22, // 0b00100010
    Get_Channel_Info_Response =     0x23, // 0b00100011
    List_All_Channels_Request =     0x2A, // 0b00101010
    List_All_Channels_Response =    0x2B, // 0b00101011
    Send_Message_Request =          0x30, // 0b00110000
    Send_Message_Response =         0x31, // 0b00110001
    Get_Message_Request =           0x32, // 0b00110010
    Get_Message_Response =          0x33, // 0b00110011
};

enum Status : u8 {
    // Success
    SuccessOk = 0x00,
    // Sender errors (0x40-0x7F)
    SenderInvalidVersion = 0x40,
    SenderInvalidType = 0x41,
    SenderInvalidSize = 0x42,
    SenderMalformedRequest = 0x43,
    SenderInvalidCredentials = 0x44,
    SenderNotFound = 0x45,
    SenderAlreadyExists = 0x46,
    SenderNotRegistered = 0x47,
    SenderForbidden = 0x48,
    SenderNotChannelMember = 0x49,
    // Receiver errors (0x80-0xBF)
    ReceiverInternalError = 0x80,
    ReceiverServiceUnavailable = 0x81,
    ReceiverResourceExhausted = 0x82,
    ReceiverMessageTooLarge = 0x83,
    ReceiverTimeout = 0x84,
};

enum UserStatus : u8 {
    Offline = 0x0,
    Online = 0x1,
};

// Well-known types

enum Flag : bool {
    No = 0,
    Yes = 1
};

fn formatIpv4Address(ref auto ip) {
    return std::format("{}.{}.{}.{}", ip.a, ip.b, ip.c, ip.d);
};

struct IPv4Address {
    be u8 a, b, c, d;
} [[format("formatIpv4Address")]];

fn formatEpoch(u64 seconds) {
    return std::time::format(std::time::to_local(seconds), "%Y-%m-%d %H:%M:%S");
};

using Timestamp = be u64 [[format("formatEpoch")]];

// Header

struct Header {
    Version version;
    Type type;
    Status status;
    padding[1];
    be u32 size;
};

// Body

using ServerId = be u8;
using UserId = be u8;
using ChannelId = be u8;

struct Auth {
    char username[16];
    char password[16];
};

struct Server_Registration {
    IPv4Address ip;
    ServerId id;
};

struct Server_Health_Check {
    IPv4Address ip;
    ServerId id;
};

struct Server_Activation {
    IPv4Address ip;
    ServerId id;
};

struct Get_Activated_Server {
    IPv4Address ip;
    ServerId id;
};

struct Server_Deactivation {
    IPv4Address ip;
    ServerId id;
};

struct User_Registration {
    Auth auth;
    UserId id;
};

struct Get_User_Info {
    Auth auth;
    char username[16];
    UserId userId;
};

struct Login_or_Logout {
    Auth auth;
    IPv4Address ip;
    UserStatus status;
};

struct Log {
    ServerId id;
    padding[1];
    be u16 messageLen;
    char messageArr[messageLen];
};

struct Get_Channel_Info {
    Auth auth;
    char channelname[16];
    ChannelId channelId;
    be u8 userIdLen;
    UserId userIdArr[userIdLen];
};

struct List_All_Channels {
    Auth auth;
    be u8 channelIdLen;
    ChannelId channelIdArr[channelIdLen];
};

struct Send_Message {
    Auth auth;
    Timestamp timestamp;
    be u16 messageLen;
    ChannelId channelId;
    char messageArr[messageLen];
};

struct Get_Message {
    Auth auth;
    Timestamp timestamp;
    be u16 messageLen;
    ChannelId channelId;
    UserId senderId;
    char messageArr[messageLen];
};

struct Data {
    Header header;
    std::assert(header.version == Version::V2, "Version must be V2 (0x02)");

    match (header.type) {
        (Type::Server_Registration_Request): Server_Registration body;
        (Type::Server_Registration_Response): Server_Registration body;
        (Type::Server_Health_Check_Request): Server_Health_Check body;
        (Type::Server_Health_Check_Response): Server_Health_Check body;
        (Type::Server_Activation_Request): Server_Activation body;
        (Type::Server_Activation_Response): Server_Activation body;
        (Type::Get_Activated_Server_Request): Get_Activated_Server body;
        (Type::Get_Activated_Server_Response): Get_Activated_Server body;
        (Type::Server_Deactivation_Request): Server_Deactivation body;
        (Type::Server_Deactivation_Response): Server_Deactivation body;
        (Type::User_Registration_Request): User_Registration body;
        (Type::User_Registration_Response): User_Registration body;
        (Type::Get_User_Info_Request): Get_User_Info body;
        (Type::Get_User_Info_Response): Get_User_Info body;
        (Type::Login_or_Logout_Request): Login_or_Logout body;
        (Type::Login_or_Logout_Response): Login_or_Logout body;
        (Type::Log_Request): Log body;
        (Type::Log_Response): Log body;
        (Type::Get_Channel_Info_Request): Get_Channel_Info body;
        (Type::Get_Channel_Info_Response): Get_Channel_Info body;
        (Type::List_All_Channels_Request): List_All_Channels body;
        (Type::List_All_Channels_Response): List_All_Channels body;
        (Type::Send_Message_Request): Send_Message body;
        (Type::Send_Message_Response): Send_Message body;
        (Type::Get_Message_Request): Get_Message body;
        (Type::Get_Message_Response): Get_Message body;
    }
};

Data data @ 0x0;
