# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_GEGLFILTER']

shared_library('motion_shadow', 'motion_shadow.c', 
  c_args : lib_args,
  dependencies : gegl,
  include_directories: inc,
  name_prefix : '',
  install: true,
  install_dir: gegl_plugin_dir,
)
