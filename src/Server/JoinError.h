#ifndef CAVEBLITZ_SERVER_JOIN_ERROR_H
#define CAVEBLITZ_SERVER_JOIN_ERROR_H

#include "../Common/ServerSentData.h"
#include <cstdint>

class JoinError : public std::runtime_error {
public:
    explicit JoinError(uint8_t error_code);
    uint8_t error_code;

    ClientFatalError to_client_fatal_error() const;
};

#endif // CAVEBLITZ_SERVER_JOIN_ERROR_H
