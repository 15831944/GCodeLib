#ifndef GCODELIB_PARSER_MANGLING_H_
#define GCODELIB_PARSER_MANGLING_H_

#include "gcodelib/Base.h"
#include <string>

namespace GCodeLib::Parser {

  class GCodeNameMangler {
   public:
    virtual ~GCodeNameMangler() = default;
    virtual std::string getProcedureName(const std::string &) const = 0;
    virtual std::string getProcedureName(int64_t) const;
    virtual std::string getStatementStart(const std::string &) const = 0;
    virtual std::string getStatementEnd(const std::string &) const = 0;
    virtual std::string getLoop(const std::string &) const = 0;
    virtual std::string getLoop(int64_t) const;
  };
}

#endif