#include "gcodelib/parser/linuxcnc/Parser.h"

namespace GCodeLib::Parser::LinuxCNC  {

  std::string GCodeLCNCMangler::getProcedureName(const std::string &proc) const {
    return "procedure_" + proc;
  }

  std::string GCodeLCNCMangler::getStatementStart(const std::string &stmt) const {
    return "stmt_" + stmt + "_start";
  }

  std::string GCodeLCNCMangler::getStatementEnd(const std::string &stmt) const {
    return "stmt_" + stmt + "_end";
  }

  std::string GCodeLCNCMangler::getLoop(const std::string &loop) const {
    return "loop_" + loop;
  }
}