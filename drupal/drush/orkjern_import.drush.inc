<?php

use Drupal\Core\Language\Language;

function orkjern_import_drush_command() {
  $items = array();
  $items['import-nodes'] = array(
    'description' => dt('Import all content from live site'),
  );
  return $items;
}

function drush_orkjern_import_import_nodes() {
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
  $nids = array();
  $has_tags = array();
  if ($code == 200) {
    // Looking good.
    try {
      $nodes = $response->json();
      // So for each one of these, try to create a node.
      foreach ($nodes as $node) {
        // Get full view of this one.
        $node_url = $url . $node['path'] . '?json';
        \Drupal::logger('my_module')->notice('Requesting ' . $node_url);
        $node_req = $client->createRequest('GET', $node_url, $hal_headers);
        $node_res = $client->send($node_req);
        $c = $node_res->getStatusCode();
        if ($c == 200) {
          // Assemble a fine piece of node.
          $json = $node_res->json();
          $edit = array(
            'uid' => 1,
            'created' => $json['created'][0]['value'],
            'type' => 'article',
            'langcode' => 'en',
            'title' => $json['title'][0]['value'],
            'promote' => 1,
          );
          $n = entity_create('node', $edit);
          $n->save();
          $nid = $n->id();
          $nids[] = $nid;
          // Add path.
          $n->get('path')->setValue(substr($node['path'], 1));
          // Add taxonomies.
          $n->get('body')->setValue(array(
            'value' => $json['body'][0]['value'],
            'format' => 'full_html',
          ));
          $taxs = array();
          if (!empty($node['field_tags_1'])) {
            $tags = explode(', ', $node['field_tags_1']);
            print_r($tags);
            foreach ($tags as $tag) {
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
          $n->get('field_tags')->setValue($taxs);
          // Add pictures..
          if (!empty($json['_links']['http://orkjern.com/rest/relation/node/article/field_image'])) {
            foreach ($json['_links']['http://orkjern.com/rest/relation/node/article/field_image'] as $img) {
              $file_url = $img['href'];
              // Transfer this please.
              $f = file_get_contents($file_url);
              $path_parts = pathinfo($file_url);
              $file_obj = file_save_data($f, 'public://' . $path_parts['basename']);
              $n->get('field_image')->setValue($file_obj->id());
            }
          }
          $n->save();
        }
        else {
          throw new Exception('BIG BIG BIG PROBLEMS');
        }
      }
      #throw new Exception('eabs');
    }
    catch (Exception $e) {
      // Ah crap. Roll back.
      foreach ($nids as $nid) {
        $n = node_load($nid);
        $n->delete();
      }
      \Drupal::logger('my_module')->error($e->getMessage());
    }
  }
  else {
    \Drupal::logger('my_module')->error('NO NODES!111');
  }
}
