--TEST--
PDO Common: Bug #39656 (Crash when calling fetch() on a PDO statment object after closeCursor())
--SKIPIF--
<?php  
if (!extension_loaded('pdo')) die('skip');
$dir = getenv('REDIR_TEST_DIR');
if (false == $dir) die('skip no driver');
require_once $dir . 'pdo_test.inc';
PDOTest::skip();
?>
--FILE--
<?php

if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

$db->exec("CREATE TABLE testtable (id INTEGER NOT NULL PRIMARY KEY, usr VARCHAR( 256 ) NOT NULL)");
$db->exec("INSERT INTO testtable (id, usr) VALUES (1, 'user')");

$stmt = $db->prepare("SELECT * FROM testtable WHERE id = ?");
$stmt->bindValue(1, 1, PDO::PARAM_INT );
$stmt->execute();
$row = $stmt->fetch();
var_dump( $row );

$stmt->execute();
$stmt->closeCursor();
$row = $stmt->fetch(); // this line will crash CLI
var_dump( $row );

$db->exec("DROP TABLE testtable");

echo "Done\n";
?>
--EXPECTF--	
array(4) {
  ["id"]=>
  string(1) "1"
  [0]=>
  string(1) "1"
  ["usr"]=>
  string(4) "user"
  [1]=>
  string(4) "user"
}
bool(false)
Done

