lib_args = [
  '-DBUILDING_GEGLPLUGIN',
  '-D_GNU_SOURCE'
]

shlib = shared_library('cstripes',
  'cstripes.c',
  c_args : lib_args,
  dependencies : [gegl, cairo, math],
  include_directories: [include_directories('.'), inc],
  name_prefix : '',
)
