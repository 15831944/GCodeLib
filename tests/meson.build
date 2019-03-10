gcodetest_source = [
  'main.cpp',
  'Error.cpp',
  'runtime/Config.cpp',
  'runtime/Interpreter.cpp',
  'runtime/IR.cpp',
  'runtime/Translator.cpp',
  'runtime/Value.cpp',
  'runtime/Runtime.cpp',
  'runtime/SourceMap.cpp',
  'runtime/Storage.cpp'
]

gcodetest = executable('gcodetest', gcodetest_source,
  include_directories : include_directories('.'),
  dependencies : GCODELIB_DEPENDENCY)
test('Unit tests', gcodetest)
