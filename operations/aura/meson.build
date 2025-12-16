# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_GEGLACTIONLINES']

shlib = shared_library('outerglow', 'outerglow.c',
  c_args : lib_args,
  dependencies : gegl,
  include_directories: inc,
  name_prefix : '',
  install: true,
  install_dir: gegl_plugin_dir,
)
