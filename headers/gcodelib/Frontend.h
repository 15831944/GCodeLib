#ifndef GCODELIB_FRONTEND_H_
#define GCODELIB_FRONTEND_H_

#include "gcodelib/Base.h"
#include "gcodelib/runtime/Translator.h"
#include "gcodelib/parser/linuxcnc/Scanner.h"
#include "gcodelib/parser/linuxcnc/Parser.h"
#include <iosfwd>

namespace GCodeLib {

  template <class Scanner, class Parser, class Mangler>
  class GCodeFrontend {
   public:
    GCodeFrontend()
      : translator(this->mangler) {}

    std::unique_ptr<Runtime::GCodeIRModule> compile(std::istream &is) {
      Scanner scanner(is);
      Parser parser(scanner, this->mangler);
      auto ast = parser.parse();
      return this->translator.translate(*ast);
    }
   private:
    Mangler mangler;
    Runtime::GCodeIRTranslator translator;
  };

  using GCodeLinuxCNC = GCodeFrontend<Parser::LinuxCNC::GCodeDefaultScanner, Parser::LinuxCNC::GCodeParser, Parser::LinuxCNC::GCodeLCNCMangler>;
}

#endif