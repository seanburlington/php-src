--TEST--
PDO Common: PDOStatement SPL iterator
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded('pdo')) die('skip');
if (!extension_loaded('SPL')) die('skip SPL not available');
if (!class_exists('IteratorIterator', false)) die('skip IteratorIterator class not present');
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

$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, val VARCHAR(10), grp VARCHAR(10))');
$db->exec('INSERT INTO test VALUES(1, \'A\', \'Group1\')'); 
$db->exec('INSERT INTO test VALUES(2, \'B\', \'Group2\')'); 
$SELECT = 'SELECT val, grp FROM test';

class Test
{
	function __construct($name = 'N/A')
	{
		echo __METHOD__ . "($name)\n";
	}
}

$stmt = $db->query($SELECT, PDO::FETCH_CLASS, 'Test', array('WOW'));

$it = new IteratorIterator($stmt); /* check if we can convert that thing */

/*** HINT: If YOU plan to do so remember not to call rewind() -> see below ***/

foreach($it as $data)
{
	var_dump($data);
}

$it->next();              /* must be allowed */
var_dump($it->current()); /* must return NULL */
var_dump($it->valid());   /* must return false */

class PDOStatementAggregate extends PDOStatement implements IteratorAggregate
{
	private function __construct()
	{
		echo __METHOD__ . "\n";
		$this->setFetchMode(PDO::FETCH_NUM);   
		/* default fetch mode is BOTH, so we see if the ctor can overwrite that */
	}

	function getIterator()
	{
		echo __METHOD__ . "\n";
		$this->execute();
		return new IteratorIterator($this, 'PDOStatement');
	}
}

$stmt = $db->prepare($SELECT, array(PDO::ATTR_STATEMENT_CLASS=>array('PDOStatementAggregate')));

foreach($stmt as $data)
{
	var_dump($data);
}

?>
--EXPECTF--
Test::__construct(WOW)
object(Test)#4 (2) {
  [u"val"]=>
  unicode(1) "A"
  [u"grp"]=>
  unicode(6) "Group1"
}
Test::__construct(WOW)
object(Test)#6 (2) {
  [u"val"]=>
  unicode(1) "B"
  [u"grp"]=>
  unicode(6) "Group2"
}
NULL
bool(false)
PDOStatementAggregate::__construct
PDOStatementAggregate::getIterator
array(2) {
  [0]=>
  unicode(1) "A"
  [1]=>
  unicode(6) "Group1"
}
array(2) {
  [0]=>
  unicode(1) "B"
  [1]=>
  unicode(6) "Group2"
}
