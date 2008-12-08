<?php

define('INCLUDE_DIR', dirname(realpath(__FILE__)));

require_once INCLUDE_DIR . '/drupal_dropins.inc';
// Load some includes that really have to be loaded first
require_once INCLUDE_DIR . '/database/query.inc';
require_once INCLUDE_DIR . '/database/database.inc';

interface NetworkData {
  public function getNetworkData();
  public function getKnownIds();
  public function getUnknownIds();
  public function getInrxnId();
}

function load_includes() {
  foreach (new RecursiveIteratorIterator(new RecursiveDirectoryIterator(INCLUDE_DIR)) as $item) {
    if ((string) $item->getPathname() == __FILE__) {
      continue;
    }
    if ($item->isFile() && in_array(pathinfo($item, PATHINFO_EXTENSION), array('inc', 'php'))) {
      require_once $item;
    }
  }
}

function introspect_spl_class($item, $doitanyway = FALSE) {
  $ignore_methods = array(
    'getLinkTarget',
    // 'getChildren',
    //'hasMethod',
    //'getMethod',
    //'hasProperty',
    //'getProperty',
    //'export',
    //'isInstance',
    //'hasConstant',
    //'getConstant',
    //'newInstance',
    //'newInstanceArgs',
    //'isSubclassOf',
    //'implementsInterface',
    //'setStaticPropertyValue',
    //'getStaticPropertyValue',
    'findRoot',
    'setCsvControl',
    'getCsvControl',
    'fgetcsv',
    'fgets',
    'flock',
    'fflush',
    'ftruncate',
    'fstat',
    'fwrite',
    'fscanf',
    'fgetss',
    'fpassthru',
    'fgetc',
    'fseek',
    'ftell',
    'seek',
    'setMaxLineLen',
    'setFlags',
    'eof',
  );
  $ignore_methods = array_merge($ignore_methods, get_class_methods('RecursiveIteratorIterator'));
  $ignore_methods = array_merge($ignore_methods, get_class_methods('ReflectionClass'));

  $rsr = array();
  foreach (get_class_methods($item) as $method) {
    if (!in_array($method, $ignore_methods) && strpos($method, '__') === FALSE) {
      $rsr['method_output'][$method] = $item->$method();
    }
    $rsr['methods'][] = $method;
  }
/*  if ($item instanceof Iterator) {
    if ($item instanceof SplFileObject ? $doitanyway : TRUE) {
      foreach ($item as $it_item) {
        $rsr['tostring'][] = $it_item instanceof Iterator ? $it_item->__toString() : $it_item;
      }
    }
  }*/
  return $rsr;
}


/* ODDDDDS AND ENDSSSSS ripped from drupal */

