project('KDBTree', 'cpp', default_options: ['cpp_std=c++17', 'buildtype=debugoptimized'])

incdir = include_directories('include')
src = [
  'src/main.cpp',
  'src/Node.cpp',
  'src/Page.cpp',
  'src/Directory.cpp',
  'src/KDBTree.cpp',
]

executable('Index', sources: src, include_directories: incdir)
