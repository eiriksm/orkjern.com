<?php
namespace Drupal\orkjern_custom\Commands;

use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\node\Entity\Node;
use Drupal\taxonomy\Entity\Vocabulary;
use Drush\Commands\DrushCommands;
use GuzzleHttp\Client;

/**
 * Custom drush commands.
 */
class OrkjernCustomCommands extends DrushCommands {

  /**
   * Client.
   *
   * @var \GuzzleHttp\Client
   */
  protected $client;

  /**
   * Entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entity_manager;

  /**
   * Constructor.
   */
  public function __construct(Client $client, EntityTypeManagerInterface $entity_manager) {
    $this->client = $client;
    $this->entity_manager = $entity_manager;
  }

  /**
   * Download all content from live site
   *
   * @command download:nodes
   * @aliases download-nodes
   */
  public function downloadNodes() {
    $this->logger->info('Starting download job');
    $url = 'http://api.orkjern.com';
    // Try to get a list of all nodes.
    $hal_headers = [
      'auth' => [
        'eiriksm',
        '7Y2ve8Bd6LwQ6M7AY9Ut',
      ],
      'headers' => [
        'Accept' => 'application/hal+json',
      ],
    ];
    $response = $this->client->get($url . '/api/node?_format=hal_json', $hal_headers);
    $code = $response->getStatusCode();
    $import_directory = 'import';
    file_prepare_directory($import_directory, FILE_CREATE_DIRECTORY);
    if ($code == 200) {
      try {
        $nodes = json_decode($response->getBody());
        $path = sprintf('./%s/node.json', $import_directory);
        file_put_contents($path, json_encode($nodes));
        $this->logger->info('Saved node.json');
        // So for each one of these, try to create a node.
        foreach ($nodes as $node) {
          // Get full view of this one.
          $node_url = $url . $node->path . '?_format=hal_json';
          $this->logger->notice('Requesting ' . $node_url);
          $node_res = $this->client->get($node_url, $hal_headers);
          $c = $node_res->getStatusCode();
          if ($c == 200) {
            $this->logger->info('Downloaded JSON for ' . $node_url);
            $json = json_decode($node_res->getBody());
            $path = sprintf('./%s/%s.json', $import_directory, substr($node->path, 1));
            $dir = pathinfo($path, PATHINFO_DIRNAME);
            file_prepare_directory($dir, FILE_CREATE_DIRECTORY);
            file_put_contents($path, json_encode($json));
          }
          else {
            $this->logger->error('Had an error downloading the json for ' . $node_url);
          }
        }
      }
      catch (Exception $e) {
        $this->logger->error('Had a horrible problem');
        $this->logger->error($e->getMessage());
        $this->logger->error($e->getTraceAsString());
      }
    }
    else {
      $this->logger->error('Could not find any nodes');
    }
  }

  /**
   * Import all content from live site.
   *
   * @command import:nodes
   * @aliases import-nodes
   */
  public function importNodes() {
    $url = 'http://api.orkjern.com';
    $this->logger->notice('Starting import of nodes');
    // Find all terms.
    $has_tags = array();
    $vocabulary = Vocabulary::load('tags');
    $terms = $this->entity_manager
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
        $this->logger->notice('Importing node with path ' . $node->path);
        // Assemble a fine piece of node.
        $json = json_decode(file_get_contents('./import/' . substr($node->path, 1) . '.json'));
        $edit = [
          'uid' => 1,
          'created' => $json->created[0]->value,
          'type' => 'article',
          'langcode' => 'en',
          'title' => $json->title[0]->value,
          'promote' => 1,
        ];
        $n = $this->entity_manager
          ->getStorage('node')
          ->create($edit);
        $n->save();
        $nid = $n->id();
        $nids[] = $nid;
        // Add path.
        $n->get('path')->setValue($node->path);
        // Add taxonomies.
        $n->get('body')->setValue([
          'value' => $json->body[0]->value,
          'format' => 'full_html',
        ]);
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
                $this->logger->notice('Creating taxonomy ' . $tag);
                $t = $this->entity_manager
                  ->getStorage('taxonomy_term')
                  ->create([
                  'name' => $tag,
                  'vid' => 'tags',
                ]);
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
              $this->logger->notice('Downloading image ' . $parts['path']);
              $path_parts = pathinfo($file_url);
              $file_obj = file_save_data($f, 'public://' . $path_parts['basename']);
              $n->get('field_image')->setValue($file_obj->id());
            }
          }
        }
        $n->save();
      }
    }
    catch (\Exception $e) {
      // Ah crap. Roll back.
      foreach ($nids as $nid) {
        $n = Node::load($nid);
        $n->delete();
      }
      $this->logger->error('Had a horrible problem');
      $this->logger->error($e->getTraceAsString());
      $this->logger->error($e->getMessage());
    }
  }

}
