<?php

define('INCLUDE_DIR', realpath(__FILE__));

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