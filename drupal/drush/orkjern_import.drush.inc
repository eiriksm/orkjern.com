<?php

use Drupal\Core\Language\Language;

function orkjern_import_drush_command() {
  $items = array();
  $items['download-nodes'] = array(
    'description' => dt('Download all content from live site'),
  );
  $items['import-nodes'] = array(
    'description' => dt('Import all content from live site'),
  );
  $items['delete-shortcuts'] = array(
    'description' => dt('Deletes all shortcuts, so we can uninstall the shortcut module.'),
  );
  return $items;
}

function drush_orkjern_import_delete_shortcuts() {
  $rows = db_query('SELECT * FROM {shortcut}');
  foreach ($rows as $row) {
    $s = entity_load('shortcut', $row->id);
    $s->delete();
  }
}

function drush_orkjern_import_download_nodes() {
  $url = 'http://orkjern.com';
  // Try to get a list of all nodes.
  $client = \Drupal::service('http_client');
  $hal_headers = array(
    'headers' => array(
      'Accept' => 'application/hal+json',
    )
  );
  $res = $client->createRequest('GET', $url . '/node?json', $hal_headers);
  $response = $client->send($res);
  $code = $response->getStatusCode();
  if ($code == 200) {
    try {
      $nodes = $response->json();
      file_put_contents('./import/node.json', json_encode($nodes));
      // So for each one of these, try to create a node.
      foreach ($nodes as $node) {
        // Get full view of this one.
        $node_url = $url . $node['path'] . '?json';
        \Drupal::logger('orkjern_import')->notice('Requesting ' . $node_url);
        $node_req = $client->createRequest('GET', $node_url, $hal_headers);
        $node_res = $client->send($node_req);
        $c = $node_res->getStatusCode();
        if ($c == 200) {
          $json = $node_res->json();
          file_put_contents('./import/' . substr($node['path'], 1) . '.json', json_encode($json));
        }
      }
    }
    catch (Exception $e) {
      \Drupal::logger('orkjern_import')->notice('Had a horrible problem');
      \Drupal::logger('orkjern_import')->notice($e->getTraceAsString());
    }
  }
  else {
    \Drupal::logger('orkjern_import')->notice('Could not find any nodes');
  }
}

function drush_orkjern_import_import_nodes() {
  $nodes = json_decode(file_get_contents('./import/node.json'));
  $nids = array();
  $has_tags = array();
  // Looking good.
  try {
    // So for each one of these, try to create a node.
    foreach ($nodes as $node) {
      $node = $node;
      // Get full view of this one.
      \Drupal::logger('my_module')->notice('Requesting ' . $node_url);
      // Assemble a fine piece of node.
      $json = json_decode(file_get_contents('./import/' . substr($node->path, 1) . '.json'));
      $edit = array(
        'uid' => 1,
        'created' => $json->created[0]->value,
        'type' => 'article',
        'langcode' => 'en',
        'title' => $json->title[0]->value,
        'promote' => 1,
      );
      $n = entity_create('node', $edit);
      $n->save();
      $nid = $n->id();
      $nids[] = $nid;
      // Add path.
      $n->get('path')->setValue(substr($node->path, 1));
      // Add taxonomies.
      $n->get('body')->setValue(array(
        'value' => $json->body[0]->value,
        'format' => 'full_html',
      ));
      $taxs = array();
      if (!empty($node->field_tags)) {
        // Parse the tags, they are hidden in markup.
        $tags = explode('<li>', $node->field_tags);
        foreach ($tags as $tag) {
          $tag = trim(strip_tags($tag));
          $length = strlen($tag);
          if ($length > 0) {
            if (!$has_tags[$tag]) {
              $t = entity_create('taxonomy_term', array(
                'name' => $tag,
                'vid' => 'tags',
              ));
              $t->save();
              $has_tags[$tag] = $t->id();
            }
            $taxs[] = $has_tags[$tag];
          }
        }
      }
      $n->get('field_tags')->setValue($taxs);
      // Add pictures..
      if (!empty($json->_links->{'http://orkjern.com/rest/relation/node/article/field_image'})) {
        foreach ($json->_links->{'http://orkjern.com/rest/relation/node/article/field_image'} as $img) {
          $file_url = $img->href;
          // Transfer this please.
          $f = file_get_contents($file_url);
          $path_parts = pathinfo($file_url);
          $file_obj = file_save_data($f, 'public://' . $path_parts['basename']);
          $n->get('field_image')->setValue($file_obj->id());
        }
      }
      $n->save();
    }
  }
  catch (Exception $e) {
    // Ah crap. Roll back.
    foreach ($nids as $nid) {
      $n = node_load($nid);
      $n->delete();
    }
    \Drupal::logger('orkjern_import')->error($e->getTraceAsString());
  }
}
