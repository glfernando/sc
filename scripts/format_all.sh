#!/bin/bash

find src/  -name *.[ch] -o -name *.cpp -o -name *.cppm | xargs clang-format -i
