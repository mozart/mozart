#!/bin/sh

case `uname -s` in
    CYGWIN*)
        pathname=`cygpath -w "$1" | sed 's|\\|/|g'`
        case "$pathname" in
            ?:* )
                device=`expr substr "$pathname" 1 1 | tr '[:upper:]' '[:lower:]'`
                pathname=`expr "$pathname" : '.\(:.*\)$'`
                pathname="$device$pathname"
                echo "$pathname"
            ;;
            *)
                echo "$pathname"
            ;;
        esac
        ;;
    *)
        echo "$1"
        ;;
esac
