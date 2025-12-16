# Build shared library
shared_library('colorremoval',
  sources: [
    'colorremoval.c',
    'gegl-buffer-cl-iterator.h',
    'opencl/noise-reduction.cl.h',
    'gegl-buffer-cl-iterator.h'
  ],
  c_args: lib_args,
  dependencies: [gegl, math],
  include_directories: [include_directories('.'), inc],
  name_prefix: '',
  install: true,
  install_dir: gegl_plugin_dir,
)
