gcodelib_source = [
  'main.cpp',
  'parser/AST.cpp',
  'parser/Error.cpp',
  'parser/Parser.cpp',
  'parser/Scanner.cpp',
  'parser/Token.cpp',
  'runtime/Error.cpp',
  'runtime/Interpreter.cpp',
  'runtime/IR.cpp',
  'runtime/Runtime.cpp',
  'runtime/Translator.cpp',
  'runtime/Value.cpp'
]

gcodelib_headers = include_directories('../headers')
GCodeLib = executable('gcodelib', gcodelib_source,
  include_directories : [gcodelib_headers])
