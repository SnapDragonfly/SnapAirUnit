#!/bin/bash

if [[ $(git diff --stat) != '' ]]; then
  echo 'dirty'
else
  echo 'clean'
fi


