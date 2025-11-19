# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_GEGLACTIONLINES']

shlib = shared_library('clay', 'clay.c',
  c_args : lib_args,
  dependencies : gegl,
  include_directories: inc,
  install: true,
  install_dir: gegl_plugin_dir,
  name_prefix : '',
)
