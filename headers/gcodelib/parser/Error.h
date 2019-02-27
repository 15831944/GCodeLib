#ifndef GCODELIB_PARSER_ERROR_H_
#define GCODELIB_PARSER_ERROR_H_

#include "gcodelib/parser/Source.h"
#include <exception>
#include <string>
#include <optional>

namespace GCodeLib::Parser {

  class GCodeParseException : public std::exception {
   public:
    GCodeParseException(const char *, const SourcePosition &);
    GCodeParseException(const std::string &, const SourcePosition &);
    GCodeParseException(const char *);
    GCodeParseException(const std::string &);

    const std::string &getMessage() const;
    std::optional<SourcePosition> getLocation() const;
    const char *what() const noexcept override;
   private:
    std::string message;
    std::optional<SourcePosition> position;
  };
}

#endif