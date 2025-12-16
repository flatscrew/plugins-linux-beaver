project('gegl-plugins-linux-beaver', 'c',
  version: '0.1.0',
  license: 'GPL-3.0-or-later'
)

fs = import('fs')

version_file = join_paths(meson.project_source_root(), 'VERSION')
app_version = fs.read(version_file).strip()

gegl = dependency('gegl-0.4', required : false)
if not gegl.found()
    gegl = dependency('gegl-0.5')
endif

cairo = dependency('cairo', required : true)
math =  meson.get_compiler('c').find_library('m', required : false)

gegl_plugin_dir = gegl.get_variable('libdir') / gegl.name()

inc = include_directories('.')

subdir('operations')