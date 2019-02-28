gcodelib_source = [
  'main.cpp',
  'Error.cpp',
  'parser/AST.cpp',
  'parser/Mangling.cpp',
  'parser/Source.cpp',
  'parser/linuxcnc/Mangling.cpp',
  'parser/linuxcnc/Parser.cpp',
  'parser/linuxcnc/Scanner.cpp',
  'parser/linuxcnc/Token.cpp',
  'parser/linuxcnc/Validator.cpp',
  'parser/reprap/Mangling.cpp',
  'parser/reprap/Parser.cpp',
  'parser/reprap/Scanner.cpp',
  'parser/reprap/Token.cpp',
  'runtime/Interpreter.cpp',
  'runtime/IR.cpp',
  'runtime/Runtime.cpp',
  'runtime/SourceMap.cpp',
  'runtime/Storage.cpp',
  'runtime/Translator.cpp',
  'runtime/Value.cpp'
]

gcodelib_headers = include_directories('../headers')
GCodeLib = executable('gcodelib', gcodelib_source,
  include_directories : [gcodelib_headers])
