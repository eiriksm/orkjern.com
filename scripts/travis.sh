#!/bin/bash
npm i phantomjs-prebuilt
./node_modules/.bin/phantomjs --webdriver=8643 &
composer install
cd web
php -d sendmail_path=`which true` ../vendor/bin/drush si minimal --db-url="mysql://$DB_USERNAME@127.0.0.1/$DATABASE" -y
../vendor/bin/drush cset system.site uuid 5ffb47de-ba4e-4ba3-8a97-3c7bcfbdfa84 -y
../vendor/bin/drush cim -y
../vendor/bin/drush download-nodes
../vendor/bin/drush import-nodes
../vendor/bin/drush runserver 127.0.0.1:8888 &
../vendor/bin/wait-for-listen 8888
../vendor/bin/wait-for-listen 8643 127.0.0.1
