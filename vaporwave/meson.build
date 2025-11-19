lib_args = [
  '-DBUILDING_GEGLPLUGIN',
  '-D_GNU_SOURCE'
]

 shared_library('vaporwave',
  'vaporwave.c',
  c_args : lib_args,
  dependencies : [gegl, cairo, math],
  include_directories: inc,
  name_prefix : '',
)
