#include "gcodelib/runtime/Error.h"

namespace GCodeLib::Runtime {

  GCodeRuntimeError::GCodeRuntimeError(const char *msg)
    : message(msg) {}
  
  GCodeRuntimeError::GCodeRuntimeError(const std::string &msg)
    : message(msg) {}

  const std::string &GCodeRuntimeError::getMessage() const {
    return this->message;
  }

  const char *GCodeRuntimeError::what() const noexcept {
    return this->message.c_str();
  }
}