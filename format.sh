#!/bin/bash
clang-format -i -style="{BasedOnStyle: google, IndentWidth: 4, ColumnLimit: 100}" main.cpp
echo "Formatted main.cpp."
