#! /bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: none
$XGETTEXT `find -name *.cpp -o -name *.h -o -name \*.qml` -o $podir/libkpimidentities6.pot
