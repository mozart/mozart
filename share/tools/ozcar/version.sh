#!/bin/sh
echo \'`date | awk '{print $2" "$3" "$7}'`\' > version.oz
