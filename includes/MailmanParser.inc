<?php

/**
 *
 * @author sdboyer
 *
 */
class MailmanArchiveParser extends SplFileInfo implements Iterator, NetworkData {
// class MailmanArchiveParser extends SplFileObject {
  protected $metadata, $fp, $mailBody, $mailHeader, $buffer;
  protected $eof = FALSE;

  public function __construct($path) {
    parent::__construct($path);
    $this->fp = fopen($this, 'r');
  }

  public function current() {
    if (!$this->metadata) {
      $this->metadata = array();
      $this->parseMailItem();
    }
    return $this;
  }

  public function getNetworkData() {
    return $this->metadata;
  }

  public function getNameType() {

  }

  public function key() {
    return sha1($this->mailHeader);
  }

  public function next() {
    $this->freeCurrent();
  }

/*  public function fgets() {
    $this->buffer = fgets($this->fp);
  }*/

  public function ftell() {
    return ftell($this->fp);
  }

  protected function getStream($ending) {
    return stream_get_line($this->fp, 0, $ending);
  }

  protected function parseMailItem() {
    $this->mailHeader = trim($this->getStream("\n\n"));
    $regex = array(
      // "/^From\ [\w\pP-]*\ at\ [\w\pP-]*\ [ ]?(.*)$/x" => "$1",
      "/^From.*\n/" => "", // Kill the whole first line, we don't need it
      "/From\: ([\w\pP-]*) at ([\w\pP-]*) \((.*)\)/m" => "$1@$2\n$3",
      "/^Date/m" => "timestamp",
      "/>[\n|\t| ]{1,3}</m" => "><",
      '/\n\t[ ]?/m' => ' ',
      // These three filter out useless crud from the ref metadata
      "/&host=@drupal\.org/m" => "",
      "/type=project&/m" => "",
      "/[&]?revcount=1[&]?/m" => "",
    );
    // preg_match('/^(?:From)(?: )?(?: [\w\pP-]*\ at.*\ [\w\pP-]*).*$/', $this->mailHeader, $date);
    preg_match('/^From [\w\pP-]* at [\w\pP-]*(.*)\n/', $this->mailHeader, $date);
    $header = preg_replace(array_keys($regex), array_values($regex), $this->mailHeader);

    $stamp = strtotime(trim(($date[1])));
    // Timestamp for Tue Nov  8 21:28:00 2005, since that's roughly when the dates start working
    if ($stamp < 1131485280) {
      $header = preg_replace('/^timestamp.*\n/m', '', $header);
      $this->metadata['timestamp'] = $stamp;
    }

    // preg_match_all("/(?<=\n)[A-Za-z-]+(?=: )/m", $this->mailHeader, $keys);
    $header = explode("\n", $header);
    $this->metadata['mail'] = array_shift($header);
    $this->metadata['name'] = array_shift($header);
    // sscanf(array_shift($header), '%s %s', $this->metadata['mail'], $this->metadata['name']);
    foreach ($header as $grouping) {
      $group_name = strtolower(str_replace('-', '_', substr($grouping, 0, strpos($grouping, ': '))));
      if (method_exists($this, 'parseGroup' . $group_name)) {
        $method = 'parseGroup' . $group_name;
        $this->$method($grouping);
      }
      else {
        $this->metadata[$group_name] = trim(strstr($grouping, ':'), ': <>');
      }
    }

    $pointer = $this->ftell();
    do  {
      // Fetch the next big block of text.
      $this->buffer .= $this->getStream("\n\nFrom");
      $pointer_loc = $this->ftell();
      // Store the (rest of the) current line's contents for later checking.
      $str = fgets($this->fp);
      // Reset the pointer to where it was before getting the line.
      fseek($this->fp, $pointer_loc);
    } while (is_string($str) && !preg_match('/^(?:From)?(?: )?[\w\pP-]*\ at.*200[5|6|7|8]$/', $str));

    if ($str === FALSE) {
      $this->eof = TRUE;
    }
    // Fracking stream_get_line() bugs.
    elseif (strpos($str, 'From') !== 0) {
      fseek($this->fp, $this->ftell() - 4);
    }
    $this->mailBody = trim($this->buffer);
  }

  protected function parseGroupReferences($grouping) {
    $nids = array();
    $references = explode('><', trim(strstr($grouping, '<'), '<>'));
    foreach ($references as $key => $ref) {
      $refs = array();
      if (!strpos($ref, '=')) {
        $this->metadata['references'][$key] = $ref;
        continue;
      }
      preg_match_all('/(?:[&]?)([\w]*)=([\w]{1,12})/', $ref, $refs[$key]);
      $this->metadata['references'][$key] = array_combine($refs[$key][1], $refs[$key][2]);
      // Handle this silly repeated nid data case crap
      if (!empty($this->metadata['references'][$key]['nid'])) {
        $nid = $this->metadata['references'][$key]['nid'];
        if (!isset($nids[$this->metadata['references'][$key]['nid']])) {
          $nids[$nid] = $key;
        }
        else {
          $this->metadata['references'][$nids[$nid]] = array_merge($this->metadata['references'][$nids[$nid]], $this->metadata['references'][$key]);
          unset($this->metadata['references'][$key]);
        }
      }
    }
  }

  protected function parseGroupDate($grouping) {
    $this->metadata['timestamp'] = strtotime(trim(strstr($grouping, ':'), ': <>'));
  }

  protected function freeCurrent() {
    $this->metadata = $this->buffer = $this->mailBody = $this->mailHeader = NULL;
  }

  public function rewind() {
    fseek($this->fp, 0);
  }

  public function valid() {
    return !$this->eof;
  }

  public function __destruct() {
    fclose($this->fp);
  }
}
