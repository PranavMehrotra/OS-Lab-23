#!/bin/bash
sed -r -n -e "/$2/!p" -e "/$2/ s/([a-zA-Z][^a-zA-Z]*)([a-zA-Z][^a-zA-Z]*)?/\u\1\l\2/gpi" < $1