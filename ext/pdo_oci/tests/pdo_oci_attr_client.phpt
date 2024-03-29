--TEST--
PDO_OCI: Attribute: Client version
--SKIPIF--
<?php
if (!extension_loaded('pdo') || !extension_loaded('pdo_oci')) die('skip not loaded');
require(dirname(__FILE__).'/../../pdo/tests/pdo_test.inc');
PDOTest::skip();
?>
--FILE--
<?php

require(dirname(__FILE__) . '/../../pdo/tests/pdo_test.inc');

$dbh = PDOTest::factory();

echo "ATTR_CLIENT_VERSION: ";
$cv = $dbh->getAttribute(PDO::ATTR_CLIENT_VERSION);
var_dump($cv);

$s = split("\.", $cv);
if ($s[0] >= 10 && count($s) > 1 && $s[1] >= 2) {
	if (count($s) != 5) {
		echo "Wrong number of values in array\nVersion was: ";
		var_dump($cv);
	} else {
		echo "Version OK, so far as can be portably checked\n";
	}
} else {
	if (count($s) != 2) {
		echo "Wrong number of values in array\nVersion was: ";
		var_dump($cv);
	} else {
		echo "Version OK, so far as can be portably checked\n";
	}
}

echo "Done\n";

?>
--EXPECTF--
ATTR_CLIENT_VERSION: unicode(%d) "%d.%s"
Version OK, so far as can be portably checked
Done
