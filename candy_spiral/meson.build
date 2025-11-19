# Plugin definition with proper install path
shared_module('candyspiral',
  'candyspiral.c',
  dependencies: [gegl, math],
  name_prefix: '',
  include_directories: inc,
  c_args: [
    '-DHAVE_CONFIG_H',
    '-DG_LOG_DOMAIN="GEGL-grok"',
    '-fPIC'
  ],
  install: true,
  install_dir: gegl_plugin_dir,
)
