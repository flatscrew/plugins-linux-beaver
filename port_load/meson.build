# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_GEGLFILTER']

shlib = shared_library('loadport', 'loadport.c', 'gegl-gio-private.h', 'gegl-plugin.h',
  c_args : lib_args,
  dependencies : gegl,
  include_directories: inc,
  name_prefix : '',
)
