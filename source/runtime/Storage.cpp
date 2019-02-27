#include "gcodelib/runtime/Storage.h"

namespace GCodeLib::Runtime {

  GCodeCascadeVariableScope::GCodeCascadeVariableScope(GCodeVariableScope *parent)
    : numbered(parent ? &parent->getNumbered() : nullptr),
      named(parent ? &parent->getNamed() : nullptr) {}
  
  GCodeDictionary<int64_t> &GCodeCascadeVariableScope::getNumbered() {
    return this->numbered;
  }

  GCodeDictionary<std::string> &GCodeCascadeVariableScope::getNamed() {
    return this->named;
  }

  GCodeCustomVariableScope::GCodeCustomVariableScope(GCodeDictionary<int64_t> &numbered, GCodeDictionary<std::string> &named)
    : numbered(numbered), named(named) {}

  GCodeDictionary<int64_t> &GCodeCustomVariableScope::getNumbered() {
    return this->numbered;
  }

  GCodeDictionary<std::string> &GCodeCustomVariableScope::getNamed() {
    return this->named;
  }
}