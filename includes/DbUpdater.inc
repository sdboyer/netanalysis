<?php

class DbUpdater {
  const BY_KNOWN = 1;
  const BY_UNKNOWN = 2;

  const PRSID_NEW = 4;
  const PRSID_FOUND = 8;
  const PRSID_CONFLICT = 16;

  protected $nd, $toUpdate, $prsid, $inrxnId;
  protected $fields = array();
  protected $prsidMatches = array();
  // Whether or not this entry should be skipped
  // protected $skip = FALSE;
  // Whether or not this is a new prsid being generated
  protected $prsidState = 0;
  protected $update = FALSE;

  public function __construct(NetworkData $data) {
    $this->nd = $data;
    $this->fields = $data->getNetworkData();
    $this->inrxnId = $data->getInrxnId();
    $this->update = (bool) db_select('inrxn')
      ->condition('inrxnid', $data->getInrxnId())
      ->execute()->columnCount();
  }

  public function prepData() {
    if ($this->update) {
      $this->prepIds();
    }
  }

  public function hitIt() {

  }

  protected function prepIds() {
    $this->getPerson($this->nd->getKnownIds());
    if ($this->prsidState & self::PRSID_NEW) {
      // No matches found. Make sure we don't get any matches using unknowns, either.
      $this->getPerson($this->nd->getUnknownIds(), self::BY_UNKNOWN);
    }

    // We have a single match, so reverse tack and find all the alternate ids
    // that are passed in which already exist in the db
    if ($this->prsidState & self::PRSID_FOUND) {
      $ids = self::getPersonAlternateIds($this->prsid);
      foreach (array_merge($this->nd->getKnownIds(), $this->nd->getUnknownIds()) as $type => $id) {
        $match = array_search($id, $ids);
        if (!empty($ids[$type])) {
          $match = array_search($id, $ids[$type]);
        }
        if (!$match) {
          $this->fields[$type] = $id;
        }
      }
    }
    // TODO Need to put in handling for conflicts
  }

  protected function getPerson(Array $ids, $known = self::BY_KNOWN) {
    // TODO Consider making this a single large calculated query
    // TODO Also consider headdesking for an hour, EQUALLY A WASTE OF TIME
    foreach ($ids as $type => $id) {
      $this->prsidMatches[] = Database::getActiveConnection()->select($this->personTable($type))
        ->fields('prsid')
        ->condition($type, $id)
        ->execute()->fetchField();
    }

    $this->prsidMatches = array_unique($this->prsidMatches);
    switch (count($this->prsidMatches)) {
      case 0:
        $this->prsidState |= (self::PRSID_NEW | $known);
      case 1:
        $this->prsid = array_shift($this->prsidMatches);
        $this->prsidState |= (self::PRSID_FOUND | $known);
      default:
        $this->prsidState |= (self::PRSID_CONFLICT | $known);
    }
  }

  public static function getPersonAlternateIds($prsid) {
    $ids = Database::getActiveConnection()->select('person')
      ->condition('prsid', $prsid)
      ->execute()->fetchAssoc();

    $ids['nicks'] = Database::getActiveConnection()->select('person', 'p')
      ->rightJoin('nick_ids', 'ni', 'p.prsid = ni.prsid')
      ->fields('ni', array('nick'))
      ->condition('p.prsid', $prsid)
      ->execute()->fetchCol();

    $ids['mails'] = Database::getActiveConnection()->select('person', 'p')
      ->rightJoin('mail_ids', 'mi', 'p.prsid = mi.prsid')
      ->fields('mi', array('mail'))
      ->condition('p.prsid', $prsid)
      ->execute()->fetchCol();

    return $ids;
  }

/*    $result = Database::getActiveConnection()->select('person', 'p')
      ->fields('p', array(
        // 'prsid',
        'do_uid',
        'do_name',
        'real_name',
        'cvs_name'))
      ->leftJoin('nick_ids', 'ni')
      ->fields('ni', array('nick'))
      ->leftJoin('mail_ids', 'mi')
      ->fields('mi', array('mail'))
      ->condition('p.prsid', $this->prsid)
      ->execute();
  }*/

  protected function personTable($field) {
    switch ($field) {
      case 'nick':
        return 'nick_ids';
      case 'mail':
        return 'mail_ids';
      case 'do_name':
      case 'real_name':
      case 'cvs_name':
        return 'person';
    }
  }
}