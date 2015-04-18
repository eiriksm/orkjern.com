#!/bin/bash
cd drupal
drush si --db-url="mysql://$DB_USERNAME@127.0.0.1/$DATABASE"
drush cset system.site uuid b1a21ab8-84c4-4028-bb09-9f3f9935cb51 -y
drush pm-uninstall contact -y  
drush delete-shortcuts
drush cim staging -y
drush import-nodes
