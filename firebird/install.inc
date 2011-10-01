<?php
// $Id$

// Firebird/Interbase specific install functions

class DatabaseTasks_firebird extends DatabaseTasks {
  protected $pdoDriver = 'firebird';
  public function name() {
    return 'Firebird';
  }
}

