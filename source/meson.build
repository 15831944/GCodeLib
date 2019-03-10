gcodelib_source = [
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
  'runtime/Config.cpp',
  'runtime/Interpreter.cpp',
  'runtime/IR.cpp',
  'runtime/Runtime.cpp',
  'runtime/SourceMap.cpp',
  'runtime/Storage.cpp',
  'runtime/Translator.cpp',
  'runtime/Value.cpp'
]

gcodelib_headers = include_directories('../headers')
GCodeLib = static_library('gcodelib', gcodelib_source,
  include_directories : [gcodelib_headers])

GCodeMain = executable('gcodemain', 'main.cpp',
  include_directories : [gcodelib_headers],
  link_with : GCodeLib)

GCODELIB_DEPENDENCY = declare_dependency(link_with : GCodeLib,
  include_directories : [gcodelib_headers])
