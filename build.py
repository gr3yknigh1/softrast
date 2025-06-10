from __future__ import annotations

from os.path import dirname, realpath, sep

from hbuild import *

project_folder = dirname(realpath(__file__))
assets_folder  = sep.join([project_folder, "assets"])


softrast = add_executable("softrast", sources=[
    "softrast.cpp",
])

add_package("sofrast", targets=[
    softrast,
])
