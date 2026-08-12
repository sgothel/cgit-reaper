#!/bin/sh

touch /var/log/cgit-reaper.log
chown -v webrunner:webrunner /var/log/cgit-reaper.log

cp -v cgit-reaper /srv/www/cgit/cgit-reaper
chown -v webrunner:webrunner /srv/www/cgit/cgit-reaper
