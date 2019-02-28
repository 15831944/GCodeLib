#ifndef GCODELIB_RUNTIME_ERROR_H_
#define GCODELIB_RUNTIME_ERROR_H_

#include "gcodelib/Error.h"

namespace GCodeLib::Runtime {

  class GCodeRuntimeError : public GCodeLibException {
   public:
    using GCodeLibException::GCodeLibException;
  };
}

#endif