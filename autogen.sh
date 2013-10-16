#!/bin/sh
mkdir -p m4 || exit $?
autoreconf --install || exit $?
