--TEST--
PDO Common: PDO::ATTR_CASE
--SKIPIF--
<?php # vim:ft=php
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

$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, val VARCHAR(10))');
$db->exec("INSERT INTO test VALUES(1, 'A')");
$db->exec("INSERT INTO test VALUES(2, 'B')");
$db->exec("INSERT INTO test VALUES(3, 'C')");

// Lower case columns
$db->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER);
$stmt = $db->prepare('SELECT * from test');
$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
$stmt->closeCursor();

// Upper case columns
$db->setAttribute(PDO::ATTR_CASE, PDO::CASE_UPPER);
$stmt = $db->prepare('SELECT * from test');
$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
$stmt->closeCursor();

?>
--EXPECT--
array(3) {
  [0]=>
  array(2) {
    [u"id"]=>
    unicode(1) "1"
    [u"val"]=>
    unicode(1) "A"
  }
  [1]=>
  array(2) {
    [u"id"]=>
    unicode(1) "2"
    [u"val"]=>
    unicode(1) "B"
  }
  [2]=>
  array(2) {
    [u"id"]=>
    unicode(1) "3"
    [u"val"]=>
    unicode(1) "C"
  }
}
array(3) {
  [0]=>
  array(2) {
    [u"ID"]=>
    unicode(1) "1"
    [u"VAL"]=>
    unicode(1) "A"
  }
  [1]=>
  array(2) {
    [u"ID"]=>
    unicode(1) "2"
    [u"VAL"]=>
    unicode(1) "B"
  }
  [2]=>
  array(2) {
    [u"ID"]=>
    unicode(1) "3"
    [u"VAL"]=>
    unicode(1) "C"
  }
}
