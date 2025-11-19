# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_ANOTHERGEGLPLUGIN']

shared_library('gradient-map-port', 'gradient-map-port.c',
  c_args : lib_args,
  dependencies : gegl,
  include_directories: inc,
  name_prefix : '',
)
