#!/bin/sh

exefile=$1

if [ -z "${exefile}" ] ; then
    echo "Usage: $0 <exefile>"
    return 0
fi
bname=$(basename ${exefile})

mkdir -pv /var/run/${bname}
chown -v webrunner:webrunner /var/run/${bname}

touch /var/log/${bname}.log
chown -v webrunner:webrunner /var/log/${bname}.log

cp -v ${exefile} /srv/www/cgit/${bname}
chown -v webrunner:webrunner /srv/www/cgit/${bname}
