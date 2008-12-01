<?php

class MailmanArchiveParser extends SplFileObject {
  public function __construct($path) {
    parent::__construct($path);
  }
}