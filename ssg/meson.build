# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_GEGLACTIONLINES']

shared_library('ssg', 'ssg.c',
  c_args : lib_args,
  dependencies : gegl,
  include_directories: inc,
  name_prefix : '',
)
