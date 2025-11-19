# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_GEGLFILTER']

shlib = shared_library('chiselbevel', 'chiselbevel.c',
  c_args : lib_args,
  dependencies : gegl,
  include_directories: inc,
  name_prefix : '',
)

# Make this library usable as a Meson subproject.
stroke_dep = declare_dependency(
  include_directories: inc,
  link_with : shlib)

