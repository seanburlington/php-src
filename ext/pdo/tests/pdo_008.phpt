--TEST--
PDO Common: PDO::FETCH_UNIQUE conflict
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

$db->exec('CREATE TABLE test(id CHAR(1) NOT NULL PRIMARY KEY, val VARCHAR(10))');
$db->exec("INSERT INTO test VALUES('A', 'A')");
$db->exec("INSERT INTO test VALUES('B', 'A')");
$db->exec("INSERT INTO test VALUES('C', 'C')");

$stmt = $db->prepare('SELECT val, id from test');

$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_NUM|PDO::FETCH_UNIQUE));

?>
--EXPECT--
array(2) {
  [u"A"]=>
  array(1) {
    [0]=>
    unicode(1) "B"
  }
  [u"C"]=>
  array(1) {
    [0]=>
    unicode(1) "C"
  }
}
