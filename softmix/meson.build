# Compiler arguments
lib_args = ['-DBUILDING_softmix']

# Build shared library
shared_library('softmixed',
  sources: [
    'softmix.c',
    'gegl-buffer-cl-iterator.h',
    'gegl-buffer-cl-iterator.h'
  ],
  c_args: lib_args,
  dependencies: [gegl, math],
  include_directories: [inc, include_directories('.')],
  name_prefix: '',
  install: true,
  install_dir: gegl_plugin_dir,
)
