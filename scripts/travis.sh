#!/bin/bash

set -euo pipefail

npm install chromedriver@2.35
git clean -f package*
./node_modules/.bin/chromedriver --port=8643 --url-base=wd/hub &
composer install
cd web
mysql -u root -proot -h 127.0.0.1 -e "CREATE DATABASE db;"
php -d sendmail_path=`which true` ../vendor/bin/drush si --existing-config --db-url="mysql://root:root@127.0.0.1:3306/db" -y
../vendor/bin/drush runserver 127.0.0.1:8888 &
../vendor/bin/wait-for-listen 8888
../vendor/bin/wait-for-listen 8643 127.0.0.1
