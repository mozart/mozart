#! /bin/sh
url=$1
shift
$OZEMULATOR -u $url -- "$@"
