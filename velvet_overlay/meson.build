# Compiler arguments
lib_args = ['-DBUILDING_GEGLPLUGIN']

# Build shared library
shared_library('velvetoverlay',
  sources: [
    'velvetoverlay.c',
    'gegl-buffer-cl-iterator.h',
    'gegl-buffer-cl-iterator.h'
  ],
  c_args: lib_args,
  dependencies: [gegl, math],
  include_directories: [inc, include_directories('.')],
  name_prefix: '',
  install: true,
  install_dir: get_option('libdir') / 'gegl-0.4'
)
