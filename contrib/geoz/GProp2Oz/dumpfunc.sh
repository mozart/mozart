#!/bin/sh
#nm -C $1 | cut -b10- | grep ". Gecode::[[:alpha:]][^(:<]*(.*)$"
nm -C $1 | cut -b10- | grep "^T" | cut -b3- | grep "^Gecode::[[:alpha:]][^(:<]*(.*)$"
