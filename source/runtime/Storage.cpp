#include "gcodelib/runtime/Storage.h"

namespace GCodeLib {

  GCodeCascadeVariableScope::GCodeCascadeVariableScope(GCodeVariableScope *parent)
    : numbered(parent ? &parent->getNumbered() : nullptr),
      named(parent ? &parent->getNamed() : nullptr) {}
  
  GCodeDictionary<int64_t> &GCodeCascadeVariableScope::getNumbered() {
    return this->numbered;
  }

  GCodeDictionary<std::string> &GCodeCascadeVariableScope::getNamed() {
    return this->named;
  }
}