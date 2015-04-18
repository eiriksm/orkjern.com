#!/bin/bash
cd drupal
php -d sendmail_path=`which true` ~/.composer/vendor/bin/drush.php si --db-url="mysql://$DB_USERNAME@127.0.0.1/$DATABASE" --keep-config -y
drush cset system.site uuid b1a21ab8-84c4-4028-bb09-9f3f9935cb51 -y
drush pm-uninstall contact -y
drush delete-shortcuts
echo "\$config_directories['staging'] = 'config/staging';" | sudo tee -a sites/default/settings.php
drush cim staging -y
mkdir -p import/node
drush download-nodes
drush import-nodes
drush runserver 127.0.0.1:8080 &
until netstat -an 2>/dev/null | grep '8080.*LISTEN'; do true; done
