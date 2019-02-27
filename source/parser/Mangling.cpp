#include "gcodelib/parser/Mangling.h"

namespace GCodeLib::Parser {

  std::string GCodeNameMangler::getProcedureName(int64_t id) const {
    std::string label = std::to_string(id);
    return this->getProcedureName(label);
  }

  std::string GCodeNameMangler::getLoop(int64_t id) const {
    std::string label = std::to_string(id);
    return this->getLoop(label);
  }
}