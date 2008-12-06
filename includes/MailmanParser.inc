<?php

/**
 *
 * @author sdboyer
 *
 */
class MailmanArchiveParser extends SplFileInfo implements Iterator {
// class MailmanArchiveParser extends SplFileObject {
  protected $cur, $fp, $mailBody, $mailHeader, $buffer;

  public function __construct($path) {
    parent::__construct($path);
    $this->fp = fopen($this, 'r');
  }

  public function current() {
    if (!$this->cur) {
      $this->cur = array();
      $this->parseMailItem();
    }
    return $this;
  }

  public function getNetworkData() {
    return $this->cur;
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
      $this->cur['timestamp'] = $stamp;
    }

    // preg_match_all("/(?<=\n)[A-Za-z-]+(?=: )/m", $this->mailHeader, $keys);
    $header = explode("\n", $header);
    $this->cur['mail'] = array_shift($header);
    $this->cur['name'] = array_shift($header);
    // sscanf(array_shift($header), '%s %s', $this->cur['mail'], $this->cur['name']);
    foreach ($header as $grouping) {
      $group_name = strtolower(str_replace('-', '_', substr($grouping, 0, strpos($grouping, ': '))));

      if ($group_name != 'references') {
        $this->cur[$group_name] = trim(strstr($grouping, ':'), ': <>');
        continue;
      }

      // Message references take more handling
      $nids = array();
      $references = explode('><', trim(strstr($grouping, '<'), '<>'));
      foreach ($references as $key => $ref) {
        $refs = array();
        if (!strpos($ref, '=')) {
          $this->cur['references'][$key] = $ref;
          continue;
        }
        preg_match_all('/(?:[&]?)([\w]*)=([\w]{1,12})/', $ref, $refs[$key]);
        $this->cur['references'][$key] = array_combine($refs[$key][1], $refs[$key][2]);
        // Handle this silly repeated nid data case crap
        if (!empty($this->cur['references'][$key]['nid'])) {
          $nid = $this->cur['references'][$key]['nid'];
          if (!isset($nids[$this->cur['references'][$key]['nid']])) {
            $nids[$nid] = $key;
          }
          else {
            $this->cur['references'][$nids[$nid]] = array_merge($this->cur['references'][$nids[$nid]], $this->cur['references'][$key]);
            unset($this->cur['references'][$key]);
          }
        }
      }
    }

    $pointer = $this->ftell();
    do  {
      $this->buffer .= $this->getStream("\n\nFrom");
      $pointer_loc = $this->ftell();
      $str = fgets($this->fp);
      fseek($this->fp, $pointer_loc);
    } while (!preg_match('/^(?:From)?(?: )?[\w\pP-]*\ at.*200[5|6|7|8]$/', $str));
    // Fracking stream_get_line() bugs.
    if (strpos($str, 'From') !== 0) {
      fseek($this->fp, $this->ftell() - 4);
    }

/*    for ($this->buffer = $this->getStream("\n\nFrom"), $pointer_loc = $this->ftell(), $str = fgets($this->fp);
         !preg_match('/^(?:From\ )?[\w\pP-]*\ at.*200[5|6|7|8]$/', $str);
         $this->buffer .= $this->getStream("\n\nFrom")) {

      fseek($this->fp, $pointer_loc);
    }*/
    $this->mailBody = trim($this->buffer);
  }

  protected function freeCurrent() {
    $this->cur = $this->buffer = $this->mailBody = $this->mailHeader = NULL;
  }

  public function rewind() {
    fseek($this->fp, 0);
  }

  public function valid() {
    return !feof($this->fp);
  }

  public function __destruct() {
    fclose($this->fp);
  }
}
