# These arguments are only used to build the shared library
lib_args = [
  '-DBUILDING_GEGLPLUGIN',
  '-D_GNU_SOURCE'  # Enable GNU extensions including sincos
]

shlib = shared_library('circlepatterns', 
  'circlepatterns.c',
  'gegl-operation-area-filter.h', 
  'gegl-operation.h',
  c_args : lib_args,
  include_directories: [include_directories('.'), inc],
  dependencies : [gegl, math],
  name_prefix : '',
)
