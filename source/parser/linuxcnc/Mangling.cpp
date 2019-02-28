/*
  SPDX short identifier: MIT
  Copyright 2019 Jevgēnijs Protopopovs
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

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