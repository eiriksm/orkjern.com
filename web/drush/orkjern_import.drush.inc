<?php

use Drupal\Core\Language\Language;
use Drupal\taxonomy\Entity\Vocabulary;
use GuzzleHttp\ClientInterface;

/**
 * Implements hook_drush_commend().
 */
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

/**
 * Command callback to delete shortcuts.
 */
function drush_orkjern_import_delete_shortcuts() {
  $rows = db_query('SELECT * FROM {shortcut}');
  foreach ($rows as $row) {
    $s = entity_load('shortcut', $row->id);
    $s->delete();
  }
}

/**
 * Command callback to download nodes from the live site.
 */
function drush_orkjern_import_download_nodes() {
  drush_log('Starting download job', 'success');
  $url = 'http://api.orkjern.com';
  // Try to get a list of all nodes.
  $client = \Drupal::service('http_client');
  $hal_headers = [
    'headers' => [
      'Accept' => 'application/hal+json',
    ],
  ];
  $response = $client->get($url . '/api/node?_format=hal_json', $hal_headers);
  $code = $response->getStatusCode();
  $import_directory = 'import';
  file_prepare_directory($import_directory, FILE_CREATE_DIRECTORY);
  if ($code == 200) {
    try {
      $nodes = json_decode($response->getBody());
      $path = sprintf('./%s/node.json', $import_directory);
      file_put_contents($path, json_encode($nodes));
      drush_log('Saved node.json', 'success');
      // So for each one of these, try to create a node.
      foreach ($nodes as $node) {
        // Get full view of this one.
        $node_url = $url . $node->path . '?_format=hal_json';
        \Drupal::logger('orkjern_import')->notice('Requesting ' . $node_url);
        $node_res = $client->get($node_url, $hal_headers);
        $c = $node_res->getStatusCode();
        if ($c == 200) {
          drush_log('Downloaded JSON for ' . $node_url, 'success');
          $json = json_decode($node_res->getBody());
          $path = sprintf('./%s/%s.json', $import_directory, substr($node->path, 1));
          $dir = pathinfo($path, PATHINFO_DIRNAME);
          file_prepare_directory($dir, FILE_CREATE_DIRECTORY);
          file_put_contents($path, json_encode($json));
        }
        else {
          drush_log('Had an error downloading the json for ' . $node_url, 'error');
        }
      }
    }
    catch (Exception $e) {
      drush_log('Had a horrible problem', 'error');
      drush_log($e->getMessage(), 'error');
      drush_log($e->getTraceAsString(), 'error');
    }
  }
  else {
    drush_log('Could not find any nodes', 'error');
  }
}

/**
 * Command callback for the importing of the downloaded nodes.
 */
function drush_orkjern_import_import_nodes() {
  $url = 'http://api.orkjern.com';
  drush_log('Starting import of nodes', 'success');
  // Find all terms.
  $has_tags = array();
  $vocabulary = Vocabulary::load('tags');
  $terms = \Drupal::entityManager()
    ->getStorage('taxonomy_term')
    ->loadTree($vocabulary->id(), 0, NULL, TRUE);
  foreach ($terms as $term) {
    $has_tags[$term->label()] = $term->id();
  }
  $nodes = json_decode(file_get_contents('./import/node.json'));
  $nids = array();
  // Looking good.
  try {
    // So for each one of these, try to create a node.
    foreach ($nodes as $node) {
      // Get full view of this one.
      drush_log('Importing node with path ' . $node->path, 'success');
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
      $n->get('path')->setValue($node->path);
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
          if ($tag == 'Tags:') {
            continue;
          }
          $length = strlen($tag);
          if ($length > 0) {
            if (!$has_tags[$tag]) {
              drush_log('Creating taxonomy ' . $tag, 'success');
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
      // Add pictures. In a horrible hacky way.
      foreach ($json->_links as $key => $relations) {
        if (!strpos($key, 'field_image')) {
          continue;
        }
        if (!empty($json->_links->{$key})) {
          foreach ($json->_links->{$key} as $img) {
            $file_url = $img->href;
            // Transfer this please.
            $parts = parse_url($file_url);
            $f = file_get_contents($url . $parts['path']);
            drush_log('Downloading image ' . $parts['path'], 'success');
            $path_parts = pathinfo($file_url);
            $file_obj = file_save_data($f, 'public://' . $path_parts['basename']);
            $n->get('field_image')->setValue($file_obj->id());
          }
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
    drush_log('Had a horrible problem', 'error');
    drush_log($e->getTraceAsString(), 'error');
    drush_log($e->getMessage(), 'error');
  }
}
