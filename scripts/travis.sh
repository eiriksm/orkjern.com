#!/bin/bash
composer install
cd web
php -d sendmail_path=`which true` ../vendor/bin/drush.php si minimal --db-url="mysql://$DB_USERNAME@127.0.0.1/$DATABASE" -y
../vendor/bin/drush cset system.site uuid 5ffb47de-ba4e-4ba3-8a97-3c7bcfbdfa84 -y
../vendor/bin/drush cim -y
../vendor/bin/drush download-nodes
../vendor/bin/drush import-nodes
../vendor/bin/drush runserver 127.0.0.1:8080 &
echo "waiting for webserver..."
sleep 8;
