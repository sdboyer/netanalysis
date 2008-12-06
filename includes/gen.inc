<?php

define('INCLUDE_DIR', dirname(realpath(__FILE__)));

// Load some includes that really have to be loaded first
require_once INCLUDE_DIR . '/database/query.inc';
require_once INCLUDE_DIR . '/database/database.inc';





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
  global $db_prefix, $db_type, $databases;
  $databases = array(
    'default' => array(
      'default' => array(
        'driver'    => 'mysql',
        'database'  => 'netanalysis',
        'username'  => 'dev',
        'password'  => 'dev',
        'host'      =>  'localhost',
        'port'      =>  '',
      ),
    ),
  );

  $db_prefix = '';
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

/**
 * Encode special characters in a plain-text string for display as HTML.
 *
 * Uses drupal_validate_utf8 to prevent cross site scripting attacks on
 * Internet Explorer 6.
 */
function check_plain($text) {
  return drupal_validate_utf8($text) ? htmlspecialchars($text, ENT_QUOTES) : '';
}

/**
 * Checks whether a string is valid UTF-8.
 *
 * All functions designed to filter input should use drupal_validate_utf8
 * to ensure they operate on valid UTF-8 strings to prevent bypass of the
 * filter.
 *
 * When text containing an invalid UTF-8 lead byte (0xC0 - 0xFF) is presented
 * as UTF-8 to Internet Explorer 6, the program may misinterpret subsequent
 * bytes. When these subsequent bytes are HTML control characters such as
 * quotes or angle brackets, parts of the text that were deemed safe by filters
 * end up in locations that are potentially unsafe; An onerror attribute that
 * is outside of a tag, and thus deemed safe by a filter, can be interpreted
 * by the browser as if it were inside the tag.
 *
 * The function does not return FALSE for strings containing character codes
 * above U+10FFFF, even though these are prohibited by RFC 3629.
 *
 * @param $text
 *   The text to check.
 * @return
 *   TRUE if the text is valid UTF-8, FALSE if not.
 */
function drupal_validate_utf8($text) {
  if (strlen($text) == 0) {
    return TRUE;
  }
  // With the PCRE_UTF8 modifier 'u', preg_match() fails silently on strings
  // containing invalid UTF-8 byte sequences. It does not reject character
  // codes above U+10FFFF (represented by 4 or more octets), though.
  return (preg_match('/^./us', $text) == 1);
}
