#
# FILE          tasks.py
#
# AUTHORS
#               Ilya Akkuzin <gr3yknigh1@gmail.com>
#
# NOTICE        (c) Copyright 2025 by Ilya Akkuzin. All rights reserved.
#
# HOW TO USE?
#
#    * Install `hpipe`:
#
#        > python3 -m pip install git+ssh://git@github.com/gr3yknigh1/hpipe
#
#    * Call build task:
#
#        > htask build --build-type Debug
#
#    * Done!
#

from typing import Callable, Any

from os.path import exists, join, dirname, realpath
import os.path

from htask import define_task, Context
from htask import load_env, save_env, is_file_busy
from htask.progs import msvc

from hbuild import compile_project, Reporter, print_report, BuildType

F = Callable

project_folder = dirname(realpath(__file__))
output_folder  = os.path.sep.join([project_folder, "build"])

#
# Tasks:
#

@define_task(name="clean")
def clean_(c: Context, build_type=BuildType.DEBUG):
    if c.exists(glm_configuration_folder(build_type)):
        c.run(f"rmdir /S /Q {glm_configuration_folder(build_type)}")

    if c.exists(output_folder):
        c.run(f"rmdir /S /Q {output_folder}")


@define_task()
def build(c: Context, build_type=BuildType.DEBUG, clean=False, reconfigure=False):
    """Builds entire project.
    """

    if clean:
        clean_(c, build_type)

    reporter = Reporter()
    compile_project(
        c,
        build_file=join(project_folder, "build.py"),
        build_type=build_type,
        prefix=output_folder,
        reporter=reporter,
    )

    print_report(reporter)
