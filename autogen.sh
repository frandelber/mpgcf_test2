#!/bin/sh
# Regenerate the autotools build system for mpgcf.
# Copyright (C) 2026 The mpgcf Authors. LGPLv3+; see COPYING.LESSER.
set -e
mkdir -p m4 build-aux
autoreconf --install --force --verbose
echo
echo "Now run: ./configure && make && make check"
