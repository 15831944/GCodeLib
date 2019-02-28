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

#include "gcodelib/Error.h"

namespace GCodeLib {

  GCodeLibException::GCodeLibException(const char *msg, const Parser::SourcePosition &loc)
    : message(msg), position(loc) {}
  
  GCodeLibException::GCodeLibException(const std::string &msg, const Parser::SourcePosition &loc)
    : message(msg), position(loc) {}

  GCodeLibException::GCodeLibException(const char *msg)
    : message(msg), position() {}
  
  GCodeLibException::GCodeLibException(const std::string &msg)
    : message(msg), position() {}
  
  const std::string &GCodeLibException::getMessage() const {
    return this->message;
  }

  std::optional<Parser::SourcePosition> GCodeLibException::getLocation() const {
    return this->position;
  }

  const char *GCodeLibException::what() const noexcept {
    return this->message.c_str();
  }
}