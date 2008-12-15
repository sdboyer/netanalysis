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
      // ->fields('inrxn', array('inrxnid'))
      ->condition('inrxnid', $this->inrxnId)
      ->countQuery()->execute()->fetchField();
  }

  public function prepData() {
    $this->prepIds();
    if ($this->update) {

    }
    $this->prepFields();
  }

  public function hitIt() {

  }

  protected function prepIds() {
    $known = $this->nd->getKnownIds();
    $unknown = $this->nd->getUnknownIds();
    // Merge the $known over the $unknown to ensure we take the ones we're more sure of
    $all = array_merge($unknown, $known);
    $this->prsidState |= $this->getPerson($known);
    if ($this->prsidState & self::PRSID_NEW && !empty($unknown)) {
      // No matches found. Make sure we don't get any matches using unknowns, either.
      $state = $this->getPerson($unknown, self::BY_UNKNOWN);
      if ($state & self::PRSID_FOUND) {
        // TODO this is nothing atm
        $this->verifyMatch();
      }
      elseif ($state & self::PRSID_CONFLICT) {
        // TODO this is nothing atm
        $this->resolveConflict();
      }
      else {
        $this->prsidState |= $state;
        foreach ($all as $type => $id) {
          $this->fields[$type] = $id;
        }
      }
    }
    // TODO Need to put in handling for conflicts
    if ($this->prsidState & self::PRSID_FOUND) {
      // We have a single match, so reverse tack and find all the alternate ids
      // that are passed in which already exist in the db
      $ids = self::getPersonAlternateIds($this->prsid);
      foreach ($all as $type => $id) {
        $match = array_search($id, $ids);
        if (!empty($ids[$type])) {
          $match = array_search($id, $ids[$type]);
        }
        if (!$match) {
          $this->fields[$type] = $id;
        }
      }
    }
  }

  protected function prepFields() {

  }

  protected function verifyMatch() {

  }

  protected function resolveConflict() {

  }

  protected function getPerson(Array $ids, $known = self::BY_KNOWN) {
    // TODO Consider making this a single large calculated query
    // TODO Also consider headdesking for an hour, EQUALLY A WASTE OF TIME
    foreach ($ids as $type => $id) {
      $this->prsidMatches[] = db_select(self::personTable($type))
        ->fields('person', array('prsid'))
        ->condition($type, $id)
        ->execute()->fetchField();
    }

    $state = 0;
    $this->prsidMatches = array_unique(array_filter($this->prsidMatches));
    switch (count($this->prsidMatches)) {
      case 0:
        $state |= (self::PRSID_NEW | $known);
        break;
      case 1:
        $this->prsid = array_shift($this->prsidMatches);
        $state |= (self::PRSID_FOUND | $known);
        break;
      default:
        $state |= (self::PRSID_CONFLICT | $known);
        break;
    }
    return $state;
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

  public static function personTable($field) {
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