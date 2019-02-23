#include <cstdlib>
#include <iostream>
#include "gcodelib/parser/Parser.h"
#include "gcodelib/ir/Translator.h"
#include <fstream>

using namespace GCodeLib;

int main(int argc, const char **argv) {
  std::ifstream is(argv[1]);
  GCodeDefaultScanner scanner(is);
  GCodeParser parser(scanner);
  auto root = parser.parse();
  GCodeIRTranslator translator;
  auto ir = translator.translate(*root);
  if (ir) {
    std::cout << *ir << std::endl;
  }
  return EXIT_SUCCESS;
}