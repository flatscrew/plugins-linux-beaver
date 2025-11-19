# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_GEGLPLUGIN']

shared_library('fixer', 'fixer.c',
  c_args : lib_args,
  dependencies : gegl,
  include_directories: inc,
  name_prefix : '',
)

