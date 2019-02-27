#include "gcodelib/parser/Error.h"

namespace GCodeLib::Parser {

  GCodeParseException::GCodeParseException(const char *msg, const SourcePosition &loc)
    : message(msg), position(loc) {}
  
  GCodeParseException::GCodeParseException(const std::string &msg, const SourcePosition &loc)
    : message(msg), position(loc) {}

  GCodeParseException::GCodeParseException(const char *msg)
    : message(msg), position() {}
  
  GCodeParseException::GCodeParseException(const std::string &msg)
    : message(msg), position() {}
  
  const std::string &GCodeParseException::getMessage() const {
    return this->message;
  }

  std::optional<SourcePosition> GCodeParseException::getLocation() const {
    return this->position;
  }

  const char *GCodeParseException::what() const noexcept {
    return this->message.c_str();
  }
}