#ifndef ROOMMESSAGE_H
#define ROOMMESSAGE_H

#include "headers.h"

struct RoomMessage {
    std::string username;
    std::string message;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RoomMessage, username, message);
};

#endif //ROOMMESSAGE_H
