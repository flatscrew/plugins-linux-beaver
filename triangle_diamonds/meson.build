# These arguments are only used to build the shared library
lib_args = [
  '-DBUILDING_GEGLPLUGIN',
  '-D_GNU_SOURCE'  # Enable GNU extensions including sincos
]
shared_library('triangle_diamond', 
  'triangle_diamond.c',
  'gegl-operation-area-filter.h', 
  'gegl-operation.h',
  c_args : lib_args,
  dependencies : [gegl, math],
  include_directories: [inc, include_directories('.')],
  name_prefix : '',
)

