<?php

// Load the general includes file.
require_once dirname(realpath(__FILE__)) . '/includes/gen.inc';
load_includes();

$file = '/home/sdboyer/Documents/Academic/Padgett.PLSC57500/final/devarchive/2005-January.txt';

$archive = new MailmanArchiveParser($file);
$dat = introspect_spl_class($archive);

foreach ($archive as $line) {
  echo $line;
}

$i = 'break on me';