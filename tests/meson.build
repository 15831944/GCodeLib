gcodetest_source = [
  'main.cpp',
  'Error.cpp',
  'runtime/Value.cpp',
  'runtime/SourceMap.cpp',
  'runtime/Storage.cpp'
]

gcodetest = executable('gcodetest', gcodetest_source,
  include_directories : include_directories('.'),
  dependencies : GCODELIB_DEPENDENCY)
test('Unit tests', gcodetest)
