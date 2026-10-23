<?php

/**
 * @file
 * Post update functions for orkjern_custom module.
 */

/**
 * Switch the node delete action to the generic entity delete plugin.
 */
function orkjern_custom_post_update_node_delete_action_plugin() {
  // The node_delete_action plugin no longer exists in Drupal 9, so the action
  // can not be loaded as an entity. Change the raw config instead.
  $config = \Drupal::configFactory()->getEditable('system.action.node_delete_action');
  if ($config->get('plugin') === 'node_delete_action') {
    $config->set('plugin', 'entity:delete_action:node')->save();
  }
}
