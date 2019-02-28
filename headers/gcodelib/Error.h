#ifndef GCODELIB_ERROR_H_
#define GCODELIB_ERROR_H_

#include "gcodelib/parser/Source.h"
#include <exception>
#include <string>
#include <optional>

namespace GCodeLib {

  class GCodeLibException : public std::exception {
   public:
    GCodeLibException(const char *, const Parser::SourcePosition &);
    GCodeLibException(const std::string &, const Parser::SourcePosition &);
    GCodeLibException(const char *);
    GCodeLibException(const std::string &);

    const std::string &getMessage() const;
    std::optional<Parser::SourcePosition> getLocation() const;
    const char *what() const noexcept override;
   private:
    std::string message;
    std::optional<Parser::SourcePosition> position;
  };
}

#endif