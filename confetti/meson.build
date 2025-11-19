# Plugin definition with proper install path
gegl_plugin_dir = gegl.get_pkgconfig_variable('libdir') / 'gegl-0.4'

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
  install_dir: gegl_plugin_dir
)
