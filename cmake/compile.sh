#!/bin/bash

file = "../bin/sprite_editor"
if [ -f file] ; then
    rm file
fi

cmake ..
make
../bin/sprite_editor