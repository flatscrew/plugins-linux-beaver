# Compiler arguments
lib_args = ['-DBUILDING_SMOOTH']

# Build shared library
shared_library('border2',
  sources: [
    'border2.c',
    'gegl-buffer-cl-iterator.h',
  ],
  c_args: lib_args,
  dependencies: [gegl, math],
  include_directories: [include_directories('.'), inc],
  name_prefix: '',
  install: true,
  install_dir: get_option('libdir') / 'gegl-0.4'
)
