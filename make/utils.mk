# SPDX-License-Identifier: BSD-3-Clause
#
# (C) Copyright 2020, Fernando Lugo <lugo.fernando@gmail.com>

GET_LOCAL_DIR = $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
