#!/bin/bash
cd drupal
php -d sendmail_path=`which true` ~/.composer/vendor/bin/drush.php si --db-url="mysql://$DB_USERNAME@127.0.0.1/$DATABASE" --keep-config -y
drush cset system.site uuid b1a21ab8-84c4-4028-bb09-9f3f9935cb51 -y
drush pm-uninstall contact -y
drush delete-shortcuts
chmod u+w sites/default/settings.php
echo "\$config_directories['staging'] = 'config/staging';" | tee -a sites/default/settings.php
drush cim staging -y
mkdir -p import/node
drush download-nodes
drush import-nodes
drush runserver 127.0.0.1:8080 &
echo "waiting for webserver..."
sleep 8;
