#! /bin/sh

for f in `find . -name configure.in -print`; do
    echo Processing $f
    autoconf -l . $f > `expr $f : '\(.*\).in'`
    chmod +x `expr $f : '\(.*\).in'`
done
