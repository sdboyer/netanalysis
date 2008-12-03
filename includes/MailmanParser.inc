<?php

/**
 *
 * @author sdboyer
 *
 */
class MailmanArchiveParser extends SplFileInfo implements Iterator {
// class MailmanArchiveParser extends SplFileObject {
  protected $cur, $fp, $mailBody, $mailHeader, $buffer, $year, $month;

  public function __construct($path) {
    parent::__construct($path);
    $this->fp = fopen($this, 'r');
  }

  public function current() {
    if (!$this->cur) {
      $this->cur = array();
      $this->parseMailItem();
      $this->extractData();
    }
    return $this->mailBody;
    // return $this->cur;
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
      "/^From\ .*\n/" => "", // Kill the whole first line, we don't need it
      "/From\:\ ([\w\pP-]*)\ at\ ([\w\pP-]*)\ \(([\w\pP-]*)\)/m" => "$1@$2 $3",
      "/>[\n|\t|\ ]{1,3}</m" => "><",
      // These three filter out useless crud from the ref metadata
      "/&host=@drupal\.org/m" => "",
      "/type=project&/m" => "",
      "/[&]?revcount=1[&]?/m" => "",
    );
    $header = preg_replace(array_keys($regex), array_values($regex), $this->mailHeader);
    // preg_match_all("/(?<=\n)[A-Za-z-]+(?=: )/m", $this->mailHeader, $keys);
    $header = explode("\n", $header);
    sscanf(array_shift($header), '%s %s', $this->cur['mail'], $this->cur['name']);
    $this->cur['date'] = substr(array_shift($header), 6);
    $this->cur['subject'] = substr(array_shift($header), 8);
    $refs = array();
    foreach ($header as $grouping) {
      $group_name = strtolower(substr($grouping, 0, strpos($grouping, ': ')));
      $grouping = explode('><', trim(strstr($grouping, '<'), '<>'));

      $nids = array();
      foreach ($grouping as $key => $pairs) {
        $refs[$group_name] = array();
        preg_match_all('/(?:[&]?)([\w]*)=([\w]*)/', $pairs, $refs[$group_name][$key]);
        $this->cur[$group_name][$key] = array_combine($refs[$group_name][$key][1], $refs[$group_name][$key][2]);
        // Handle this silly repeated nid data case crap
        if (!empty($this->cur[$group_name][$key]['nid'])) {
          $nid = $this->cur[$group_name][$key]['nid'];
          if (!isset($nids[$this->cur[$group_name][$key]['nid']])) {
            $nids[$nid] = $key;
          }
          else {
            $this->cur[$group_name][$nids[$nid]] = array_merge($this->cur[$group_name][$nids[$nid]], $this->cur[$group_name][$key]);
            unset($this->cur[$group_name][$key]);
          }
        }
      }

        // preg_match_all('/(?:<)(?:[&]?([\w]*)=([\w]*))*(?:>)/', $grouping, $refs);
      // $this->cur[$group_name] = $$group_name;
      $i = 'break on me';
    }

    // $this->numHumanResponses = preg_match_all("/(?<=\n(?In-Reply-To|References|Message-ID): )<.*>/m", $header, $refs);
    // preg_match_all("/[&|<]([\w]*)=([\w]*)[&|>]");

    // $header = preg_replace("/From\:\ ([\w\pP-]*)\ at\ ([\w\pP-]*)\ \(([\w\pP-]*)\)/m", "$1@$2 $3", $this->mailHeader);
    // $header = preg_replace("/From\:\ ([0-9A-Za-z_-]*)\ at\ ([\pN\pL\pP]*)\/m", "$1@$2", $this->mailHeader);
    // $this->mailHeader

    for ($this->buffer .= $this->getStream();
         $pointer_loc = $this->ftell(), !preg_match('/^From [0-9A-Za-z_-]*\ at.*200{5|6|7|8}$/x', fgets($this->fp));
         $this->buffer .= $this->getStream()) {

      fseek($this->fp, $pointer_loc);
    }
    $this->mailBody = trim($this->buffer);
  }

/*  protected function extractData() {
    $text =<<<EOF
%s %s
Date: %s
Subject: %s
In-Reply-To:
EOF;
    sscanf($this->mailHeader, $text,

  }
*/
  protected function freeCurrent() {
    $this->cur = $this->mailBody = $this->mailHeader = NULL;
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
