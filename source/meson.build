gcodelib_source = [
  'main.cpp',
  'ir/Interpreter.cpp',
  'ir/IR.cpp',
  'ir/Translator.cpp',
  'parser/AST.cpp',
  'parser/Parser.cpp',
  'parser/Scanner.cpp',
  'parser/Token.cpp'
]

gcodelib_headers = include_directories('../headers')
GCodeLib = executable('gcodelib', gcodelib_source,
  include_directories : [gcodelib_headers])
