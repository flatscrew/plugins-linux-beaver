# Plugin definition with proper install path
shared_module('confetti',
  'confetti.c',
  dependencies: [gegl, math],
  include_directories: [include_directories('.'), inc],
  name_prefix: '',
  c_args: [
    '-DHAVE_CONFIG_H',
    '-DG_LOG_DOMAIN="GEGL-grok"',
    '-fPIC'
  ],
  install: true,
  install_dir: gegl_plugin_dir,
)
