<?php

function install_schemas() {
  $ret = array();
  foreach (get_all_schemas() as $tablename => $schema) {
    db_create_table($ret, $tablename, $schema);
  }
}

function install_a_schema($schema) {
  $schemas = get_all_schemas();
  db_create_table($ret, $schema, $schemas[$schema]);
}

function get_all_schemas() {
  $schema = array();
  $schema['person'] = array(
    'description' => 'Primary table keyed on the unique IDs of individual persons; exactly one record per person.',
    'fields' => array(
      'prsid' => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: unique person id.',
      ),
      'do_uid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => FALSE,
        'default' => NULL,
        'description' => "Person's drupal.org uid.",
      ),
      'do_name' => array(
        'type' => 'varchar',
        'length' => 60,
        'not null' => FALSE,
        'default' => NULL,
        'description' => "Person's drupal.org username.",
      ),
      'real_name' => array(
        'type' => 'varchar',
        'length' => 100,
        'not null' => FALSE,
        'default' => NULL,
        'description' => "Person's real, full name.",
      ),
      'cvs_name' => array(
        'type' => 'varchar',
        'length' => 32,
        'not null' => FALSE,
        'default' => NULL,
        'description' => "Person's cvs account name.",
      ),
    ),
    'unique keys' => array(
      'do_name' => array('do_name'),
      'do_uid' => array('do_uid'),
      'cvs_name' => array('cvs_name'),
    ),
    'primary key' => array('prsid'),
    'module' => 'blah',
    'name' => 'person',
  );

  $schema['mail_ids'] = array(
    'description' => 'Join table listing all possible mail ids for a person.',
    'fields' => array(
      'prsid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: unique person id.',
      ),
      'mail' => array(
        'type' => 'varchar',
        'length' => 64,
        'not null' => TRUE,
        'default' => '',
        'description' => "An email address associated with this prsid.",
      ),
    ),
    'indexes' => array(
      'prsid_join' => array('prsid', 'mail'),
    ),
    'primary key' => array('mail'),
    'module' => 'blah',
    'name' => 'mail_ids',
  );

  $schema['nick_ids'] = array(
    'description' => 'Join table listing all possible nicknames for a person.',
    'fields' => array(
      'prsid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: unique person id.',
      ),
      'nick' => array(
        'type' => 'varchar',
        'length' => 100,
        'not null' => TRUE,
        'default' => '',
        'description' => "A nickname (NOT just an online alias - a 'other' name!) for this person.",
      ),
    ),
    'indexes' => array(
      'prsid_join' => array('prsid', 'nick'),
    ),
    'primary key' => array('nick'),
    'module' => 'blah',
    'name' => 'nick_ids',
  );

  $schema['inrxn'] = array(
    'description' => 'Table with listing of all interaction records of all types, keyed by the interaction id.',
    'fields' => array(
      'iid' => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: incrementing unique interaction id.',
      ),
      'setid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Unique setting id for this interaction.',
      ),
      'inrxnid' => array(
        'type' => 'varchar',
        'length' => 32,
        'not null' => TRUE,
        'description' => 'A SHA1 hash value of some unique and reproducible identifying feature of the interaction.'
      ),
      'timestamp' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
        'description' => 'Unix timestamp of the exact date and time at which this interaction occurred.'
      ),
    ),
    'indexes' => array(
      'setid_join' => array('inrxnid', 'setid'),
      'time' => array('timestamp'),
    ),
    'unique keys' => array(
      'inrxnid' => array('inrxnid'),
    ),
    'primary key' => 'iid',
    'module' => 'blah',
    'name' => 'inrxn',
  );

  return $schema;
}
