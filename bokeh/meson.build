# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_CA']

shlib = shared_library('bokeh', 'bokeh.c',
  c_args : lib_args,
  dependencies : gegl,
  include_directories: inc,
  name_prefix : '',
  install: true,
  install_dir: gegl_plugin_dir,
)
