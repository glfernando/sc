#!/bin/bash

find src/  -name *.[ch] -o -name *.cpp -name *.cppm | xargs clang-format -i
