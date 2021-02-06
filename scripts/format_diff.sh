#!/bin/bash

git diff --name-only *.cpp *.cppm *.[ch] | xargs clang-format -i
