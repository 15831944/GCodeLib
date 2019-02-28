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