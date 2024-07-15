#/bin/sh

awk 'BEGIN { FS = "[\"\(\),]*" } \
/config_set_default/ { print "set", $2, $3 }' \
src/config_default.c >doc/default.cfg.example

