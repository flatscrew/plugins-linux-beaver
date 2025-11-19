lib_args = [
  '-DBUILDING_GEGLPLUGIN',
  '-D_GNU_SOURCE'
]

shared_library('weaves',
  'weaves.c',
  c_args : lib_args,
  dependencies : [gegl, cairo, math],
  include_directories: inc,
  name_prefix : '',
)
