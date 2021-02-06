# SPDX-License-Identifier: BSD-3-Clause
#
# (C) Copyright 2020, Fernando Lugo <lugo.fernando@gmail.com>

## remove me from include list
MAKEFILE_LIST := $(lastword $(filter-out $(lastword $(MAKEFILE_LIST)), $(MAKEFILE_LIST)))

MODULE_PATH = $(GET_LOCAL_DIR)

# clean up variables
src-y =
dir-y =
module_dirs =

MODULE_CPPFLAGS =
MODULE_CFLAGS =
MODULE_CXXFLAGS =
