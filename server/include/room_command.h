#ifndef ROOMCOMMAND_H
#define ROOMCOMMAND_H

#include "headers.h"

struct RoomCommand {
    std::string operation;
    std::string roomName;
    std::string userName;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RoomCommand, operation, roomName, userName);
};


#endif //ROOMCOMMAND_H
