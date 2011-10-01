<?php
// $Id$

/**
 * @ingroup database
 * @{
 */

class InsertQuery_firebird extends InsertQuery {

  public function execute() {

    // Confirm that the user did not try to specify an identical
    //  field and default field.
    if (array_intersect($this->insertFields, $this->defaultFields)) {
      throw new PDOException('You may not specify the same field to have a value and a schema-default value.');
    }

    if (count($this->insertFields) + count($this->defaultFields) == 0) {
      return NULL;
    }

    $last_insert_id = 0;

    $max_placeholder = 0;
    $values = array();
    foreach ($this->insertValues as $insert_values) {
      foreach ($insert_values as $value) {
        $values[':db_insert_placeholder_' . $max_placeholder++] = $value;
      }
    }

    $last_insert_id = $this->connection->query((string)$this, $values, $this->queryOptions);

    // Re-initialize the values array so that we can re-use this query.
    $this->insertValues = array();

    return $last_insert_id;
  }

  public function __toString() {

    // Default fields are always placed first for consistency.
    $insert_fields = array_merge($this->defaultFields, $this->insertFields);

    $query = "INSERT INTO ". $this->table ." (" . implode(', ', $insert_fields) . ") VALUES ";

    $max_placeholder = 0;
    $values = array();
    if (count($this->insertValues)) {
      foreach ($this->insertValues as $insert_values) {
        $placeholders = array();

        // Default fields aren't really placeholders, but this is the most convenient
        // way to handle them.
        $placeholders = array_pad($placeholders, count($this->defaultFields), 'default');

        $new_placeholder = $max_placeholder + count($insert_values);
        for ($i = $max_placeholder; $i < $new_placeholder; ++$i) {
          $placeholders[] = ':db_insert_placeholder_'. $i;
        }
        $max_placeholder = $new_placeholder;
        $values[] = '('. implode(', ', $placeholders) .')';
      }
    }
    else {
      // If there are no values, then this is a default-only query. We still need to handle that.
      $placeholders = array_fill(0, count($this->defaultFields), 'default');
      $values[] = '(' . implode(', ', $placeholders) .')';
    }

    $query .= implode(', ', $values);

    return $query;
  }
}

/**
 * Firebird/Interbase specific implementation of UpdateQuery.
 */
class UpdateQuery_firebird extends UpdateQuery {

  public function execute() {
    return parent::execute();
  }

}

/**
 * @} End of "ingroup database".
 */
