# These arguments are only used to build the shared library
lib_args = [
  '-DBUILDING_GEGLPLUGIN',
  '-D_GNU_SOURCE'  # Enable GNU extensions including sincos
]

shlib = shared_library('checkers', 
  'checkers.c',
  'gegl-operation-area-filter.h', 
  c_args : lib_args,
  dependencies : [gegl, math],
  include_directories: [include_directories('.'), inc],
  name_prefix : '',
  install: true,
  install_dir: gegl_plugin_dir,
)