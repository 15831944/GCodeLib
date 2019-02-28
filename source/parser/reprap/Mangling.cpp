#include "gcodelib/parser/reprap/Parser.h"

namespace GCodeLib::Parser::RepRap  {

  std::string GCodeRepRapMangler::getProcedureName(const std::string &proc) const {
    return "procedure_" + proc;
  }

  std::string GCodeRepRapMangler::getStatementStart(const std::string &stmt) const {
    return "stmt_" + stmt + "_start";
  }

  std::string GCodeRepRapMangler::getStatementEnd(const std::string &stmt) const {
    return "stmt_" + stmt + "_end";
  }

  std::string GCodeRepRapMangler::getLoop(const std::string &loop) const {
    return "loop_" + loop;
  }
}