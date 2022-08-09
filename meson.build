# Usage:
# meson setup builddir
# cd builddir

project('dwgui', 'c',
  version : '0.0.2',
  default_options : ['warning_level=3'])
  
gtkdeps = [dependency('gtk+-3.0'),
meson.get_compiler('c').find_library('m', required: false),
]

gnome = import('gnome')

asresources = gnome.compile_resources(
    'as-resources', 'src/gresources.xml',
    source_dir: './',
    c_name: 'as'
)

subdir('src/')


executable('dwgui',
           [src, asresources], 
           dependencies: gtkdeps,
           install : true)
