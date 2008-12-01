<?php

define('INCLUDE_DIR', dirname(realpath(__FILE__)));

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

function conf_init() {

}

function introspect_spl_class($item) {
  $ignore_methods = array(
    'getLinkTarget',
    // 'getChildren',
    'hasMethod',
    'getMethod',
    'hasProperty',
    'getProperty',
    'export',
    'isInstance',
    'hasConstant',
    'getConstant',
    'newInstance',
    'newInstanceArgs',
    'isSubclassOf',
    'implementsInterface',
    'setStaticPropertyValue',
    'getStaticPropertyValue',
    'findRoot',
  );

  $rsr = array();
  foreach (get_class_methods($item) as $method) {
    if (in_array($method, $ignore_methods) || is_int(strpos($method, '__'))) {
      continue;
    }
    $rsr['methods'][$method] = $item->$method();
  }
  if ($item instanceof Iterator) {
    foreach ($item as $it_item) {
      $rsr['tostring'][] = $it_item->__toString();
    }
  }
  return $rsr;
}