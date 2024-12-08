#!/usr/bin/env bash

if [ -x "$(command -v postfix)" ]; then
    echo "Your server has been accessed! \
         by USER: $USER on HOST: $(hostname) at TIME: $(date)" | mail -s \
         "Server Accessed!" \
         ahmed15-5827@diu.edu.bd
fi
