# Plugin definition with proper install path
gegl_plugin_dir = gegl.get_pkgconfig_variable('libdir') / 'gegl-0.4'

shared_module('candyspiral',
  'candyspiral.c',
  dependencies: [gegl, math],
  name_prefix: '',
  include_directories: inc,
  c_args: [
    '-DHAVE_CONFIG_H',
    '-DG_LOG_DOMAIN="GEGL-grok"',
    '-fPIC'
  ]
)
