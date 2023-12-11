#!/bin/bash

set -euo pipefail

npm install chromedriver@2.35
./node_modules/.bin/chromedriver --port=8643 --url-base=wd/hub &
composer install
cd web
mysql -u root -proot -h db -e "CREATE DATABASE db;"
php -d sendmail_path=`which true` ../vendor/bin/drush si minimal --db-url="mysql://root:root@db:3306/db" -y
../vendor/bin/drush cset system.site uuid 5ffb47de-ba4e-4ba3-8a97-3c7bcfbdfa84 -y
../vendor/bin/drush cim -y
../vendor/bin/drush download-nodes
../vendor/bin/drush import-nodes
../vendor/bin/drush cr
../vendor/bin/drush runserver 127.0.0.1:8888 &
../vendor/bin/wait-for-listen 8888
../vendor/bin/wait-for-listen 8643 127.0.0.1
