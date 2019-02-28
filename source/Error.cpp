#include "gcodelib/Error.h"

namespace GCodeLib {

  GCodeLibException::GCodeLibException(const char *msg, const Parser::SourcePosition &loc)
    : message(msg), position(loc) {}
  
  GCodeLibException::GCodeLibException(const std::string &msg, const Parser::SourcePosition &loc)
    : message(msg), position(loc) {}

  GCodeLibException::GCodeLibException(const char *msg)
    : message(msg), position() {}
  
  GCodeLibException::GCodeLibException(const std::string &msg)
    : message(msg), position() {}
  
  const std::string &GCodeLibException::getMessage() const {
    return this->message;
  }

  std::optional<Parser::SourcePosition> GCodeLibException::getLocation() const {
    return this->position;
  }

  const char *GCodeLibException::what() const noexcept {
    return this->message.c_str();
  }
}