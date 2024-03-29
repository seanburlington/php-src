--TEST--
SQLite3::prepare Bound Variable test
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php

require_once(dirname(__FILE__) . '/new_db.inc');
define('TIMENOW', time());

echo "Creating Table\n";
var_dump($db->exec('CREATE TABLE test (time INTEGER, id STRING)'));

echo "INSERT into table\n";
var_dump($db->exec("INSERT INTO test (time, id) VALUES (" . TIMENOW . ", 'a')"));
var_dump($db->exec("INSERT INTO test (time, id) VALUES (" . TIMENOW . ", 'b')"));

echo "SELECTING results\n";
$stmt = $db->prepare("SELECT * FROM test WHERE id = ? ORDER BY id ASC");
$foo = 'c';
echo "BINDING Parameter\n";
var_dump($stmt->bindParam(1, $foo, SQLITE3_TEXT));
$foo = 'a';
$results = $stmt->execute();
while ($result = $results->fetchArray(SQLITE3_NUM))
{
	var_dump($result);
}
$results->finalize();

echo "Closing database\n";
var_dump($db->close());
echo "Done\n";
?>
--EXPECTF--
Creating Table
bool(true)
INSERT into table
bool(true)
bool(true)
SELECTING results
BINDING Parameter
bool(true)
array(2) {
  [0]=>
  int(%d)
  [1]=>
  unicode(1) "a"
}
Closing database
bool(true)
Done
