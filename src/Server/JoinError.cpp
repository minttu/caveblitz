#include "JoinError.h"

JoinError::JoinError(uint8_t error_code) : std::runtime_error(""), error_code(error_code) {
}

ClientFatalError JoinError::to_client_fatal_error() const {
    ClientFatalError err{};
    err.error_code = this->error_code;
    return err;
}