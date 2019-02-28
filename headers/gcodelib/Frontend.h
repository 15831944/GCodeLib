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

  template <class Scanner, class Parser, class Mangler, class Validator = Internal::EmptyValidator>
  class GCodeFrontend {
   public:
    using ScannerType = Scanner;
    using ParserType = Parser;
    using ManglerType = Mangler;
    using ValidatorType = Validator;
  
    GCodeFrontend()
      : translator(this->mangler) {}

    auto parse(std::istream &is, const std::string &tag = "") {
      Scanner scanner(is, tag);
      Parser parser(scanner, this->mangler);
      auto ast = parser.parse();
      if constexpr (!std::is_same<Validator, Internal::EmptyValidator>()) {
        this->validator.validate(*ast);
      }
      return *ast;
    }

    auto compile(std::istream &is, const std::string &tag = "") {
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