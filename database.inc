<?php
// $Id$

/**
 * @file
 * Database interface code for Firebird/Interbase database servers.
 */

/**
 * @ingroup database
 * @{
 */

class DatabaseConnection_firebird extends DatabaseConnection {
  public  $debug_query = false;
  public function __construct(array $connection_options = array()) {
    // This driver defaults to transaction support, except if explicitly passed FALSE.
    //$this->transactionSupport = !isset($connection_options['transactions']) || $connection_options['transactions'] === FALSE;
    // This driver defaults to non transaction support.
//    $this->transactionSupport = !empty($connection_option['transactions']);
    $this->transactionSupport = true;	

    // Default to TCP connection on port 3050.
    if (empty($connection_options['port'])) {
      $connection_options['port'] = 3050;
    }

    $dsn = 'firebird:host=' . $connection_options['host'] . ';port=' . $connection_options['port'] . ';dbname=' . $connection_options['database'];
    parent::__construct($dsn, $connection_options['username'], $connection_options['password'], array(
      // Force column names to upper case.
      PDO::ATTR_CASE => PDO::CASE_UPPER,
    ));
  }

  private function replaceKeywords($query) {
    static $keywords = array("VARIABLE", "VALUE", "TIMESTAMP", "POSITION", "EXTERNAL", "FILTER", "PAGES");
    // CACHE keyword will be treated more carefully.
    static $replacement = array();
    static $replace_built = FALSE;

    if (!$replace_built) {
      foreach ($keywords as $value) {
        $replacement[] = "\"". $value ."\"";
      }
      $replace_built = TRUE;
    }

    // Quote reserved words.
    $query = str_ireplace($keywords, $replacement, $query);
    // Rollback matched patterns that were part of a field name.
    $query = preg_replace("/\"(\w+)\"_/", "$1_", $query);
    $query = preg_replace("/_\"(\w+)\"/", "_$1", $query);
    // Rollback INSERT/UPDATE/DELETE "POSITION".
    $query = str_replace('INSERT "POSITION"','INSERT POSITION',$query);
    $query = str_replace('UPDATE "POSITION"','UPDATE POSITION',$query);
    $query = str_replace('DELETE "POSITION"','DELETE POSITION',$query);
    // Rollback "VALUE"S.
    $query = str_replace('"VALUE"S','VALUES',$query);
    // There is no int NULL type
    $query = str_replace('int NULL','int',$query);
    $query = str_replace('CRATE','RECREATE',$query);


    // Treat CACHE keyword.
    $query = str_replace('\"cache\"','"CACHE"',$query);

    return $query;
  }

  public function query($query, array $args = array(), $options = array()) {

    $options += $this->defaultOptions();

    // Remove {} from tablename.
    $query = preg_replace("/{(\w+)}/", "$1", $query);

    // Replace keywords.
    $query = $this->replaceKeywords( $query );
    //DEBUGGING

   if ($this->debug_query===true)
{    
    echo $query . "<br>";
    foreach( $args as $place => $arg ) {
     echo "   " . $place . " => " . $arg . "<br>";
    }
    echo "<br><br>";
}
    //END DEBUGGING

    try {
      if ($query instanceof DatabaseStatementInterface) {
        $stmt = $query;
        if ($this->debug_query===true) var_dump ($this);
	$this->startTransaction();
        $stmt->execute(NULL, $options);
	$this->commit();

      }
      else {

        $modified = $this->expandArguments($query, $args);
        $stmt = $this->prepareQuery($query, !$modified);
	$this->startTransaction();
	$stmt->execute($args, $options);
	$this->commit();
      }

      switch ($options['return']) {
        case Database::RETURN_STATEMENT:
          return $stmt;
        case Database::RETURN_AFFECTED:
          return $stmt->rowCount();
        case Database::RETURN_INSERT_ID:
          //return $this->lastInsertId($options['sequence_name']);
          return;
        case Database::RETURN_NULL:
          return;
        default:
          throw new PDOException('Invalid return directive: ' . $options['return']);
      }
    }
    catch (PDOException $e) {
      _db_check_install_needed();
      if ($options['throw_exception']) {
        if ($query instanceof DatabaseStatementInterface) {
          $query_string = $stmt->getQueryString();
        }
        else {
          $query_string = $query;
        }
        throw new PDOException($query_string . " - \n" . print_r($args,1) . $e->getMessage());
      }
      return NULL;
    }

  }

  public function queryRange($query,$from , $count, array $args = array(),array $options = array()) {
    return $this->query($query . ' LIMIT ' . $from . ', ' . $count, $args, $options);
  }


  public function queryTemporary($query, array $args = array(), array $options = array()) {
    $tablename = $this->generateTemporaryTableName();
    $this->query(preg_replace('/^SELECT/i', 'CREATE TEMPORARY TABLE {' . $tablename . '} Engine=MEMORY SELECT', $query), $args, $options);
    return $tablename;
  }

  public function driver() {
    return 'firebird';
  }

  public function databaseType() {
    return 'firebird';
  }

  public function mapConditionOperator($operator) {
    // We don't want to override any of the defaults.
    return NULL;
  }

  /**
   * @todo Remove this as soon as db_rewrite_sql() has been exterminated.
   */
  public function distinctField($table, $field, $query) {
    $field_to_select = 'DISTINCT('. $table .'.'. $field .')';
    // (?<!text) is a negative look-behind (no need to rewrite queries that already use DISTINCT).
    return preg_replace('/(SELECT.*)(?:'. $table .'\.|\s)(?<!DISTINCT\()(?<!DISTINCT\('. $table .'\.)'. $field .'(.*FROM )/AUsi', '\1 '. $field_to_select .'\2', $query);
  }
}

/**
 * @} End of "ingroup database".
 */
