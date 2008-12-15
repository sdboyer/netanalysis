<?php

// Be hacky/lazy and just use the same DRUPAL_ROOT namespace
define('DRUPAL_ROOT', dirname(realpath(__FILE__)));
// Load the general includes file.
require_once DRUPAL_ROOT . '/includes/gen.inc';
// Load up ALL the includes.
load_includes();
// Fire up the bootstrap-imitating conf_init()
conf_init();

// install_schemas();


$file = '/home/sdboyer/Documents/Academic/Padgett.PLSC57500/final/devarchive/2005-January.txt';

date_default_timezone_set('UTC');
$archive = new MailmanArchiveParser($file);
// $dat = introspect_spl_class($archive);

foreach ($archive as $item) {
  $db = new DbUpdater($item);
  $db->prepData();
}

$i = 'break on me';
