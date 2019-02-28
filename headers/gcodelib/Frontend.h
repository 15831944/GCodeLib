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

#ifndef GCODELIB_FRONTEND_H_
#define GCODELIB_FRONTEND_H_

#include "gcodelib/Base.h"
#include "gcodelib/runtime/Translator.h"
#include "gcodelib/parser/linuxcnc/LinuxCNC.h"
#include "gcodelib/parser/reprap/RepRap.h"
#include <type_traits>
#include <iosfwd>

namespace GCodeLib {

  namespace Internal {
    struct EmptyValidator {};
  }

  class GCodeCompilerFrontend {
   public:
    virtual ~GCodeCompilerFrontend() = default;
    virtual std::unique_ptr<Parser::GCodeBlock> parse(std::istream &, const std::string & = "") = 0;
    virtual std::unique_ptr<Runtime::GCodeIRModule> compile(std::istream &, const std::string & = "") = 0;
  };

  template <class Scanner, class Parser, class Mangler, class Validator = Internal::EmptyValidator>
  class GCodeFrontend : public GCodeCompilerFrontend {
   public:
    using ScannerType = Scanner;
    using ParserType = Parser;
    using ManglerType = Mangler;
    using ValidatorType = Validator;
  
    GCodeFrontend()
      : translator(this->mangler) {}

    std::unique_ptr<GCodeLib::Parser::GCodeBlock> parse(std::istream &is, const std::string &tag) override {
      Scanner scanner(is, tag);
      Parser parser(scanner, this->mangler);
      auto ast = parser.parse();
      if constexpr (!std::is_same<Validator, Internal::EmptyValidator>()) {
        this->validator.validate(*ast);
      }
      return ast;
    }

    std::unique_ptr<Runtime::GCodeIRModule> compile(std::istream &is, const std::string &tag) override {
      Scanner scanner(is, tag);
      Parser parser(scanner, this->mangler);
      auto ast = parser.parse();
      if constexpr (!std::is_same<Validator, Internal::EmptyValidator>()) {
        this->validator.validate(*ast);
      }
      return this->translator.translate(*ast);
    }
   private:
    Mangler mangler;
    Validator validator;
    Runtime::GCodeIRTranslator translator;
  };

  using GCodeLinuxCNC = GCodeFrontend<Parser::LinuxCNC::GCodeDefaultScanner, Parser::LinuxCNC::GCodeParser, Parser::LinuxCNC::GCodeLCNCMangler, Parser::LinuxCNC::GCodeLCNCValidator>;
  using GCodeRepRap = GCodeFrontend<Parser::RepRap::GCodeDefaultScanner, Parser::RepRap::GCodeParser, Parser::RepRap::GCodeRepRapMangler>;
}

#endif