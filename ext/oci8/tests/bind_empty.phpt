--TEST--
binding empty values
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php

require dirname(__FILE__).'/connect.inc';

$drop = "DROP table bind_test";
$statement = oci_parse($c, $drop);
@oci_execute($statement);

$create = "CREATE table bind_test(name VARCHAR(10))";
$statement = oci_parse($c, $create);
oci_execute($statement);


echo "Test 1\n";

$name = null;
$stmt = oci_parse($c, "UPDATE bind_test SET name=:name");
oci_bind_by_name($stmt, ":name", $name);

var_dump(oci_execute($stmt));

echo "Test 2\n";

$name = "";
$stmt = oci_parse($c, "UPDATE bind_test SET name=:name");
oci_bind_by_name($stmt, ":name", $name);

var_dump(oci_execute($stmt));

echo "Test 3\n";

$stmt = oci_parse($c, "INSERT INTO bind_test (NAME) VALUES ('abc')");
$res = oci_execute($stmt);

$stmt = oci_parse($c, "INSERT INTO bind_test (NAME) VALUES ('def')");
$res = oci_execute($stmt);

$name = null;
$stmt = oci_parse($c, "UPDATE bind_test SET name=:name WHERE NAME = 'abc'");
oci_bind_by_name($stmt, ":name", $name);

var_dump(oci_execute($stmt));

$stid = oci_parse($c, "select * from bind_test order by 1");
oci_execute($stid);
oci_fetch_all($stid, $res);
var_dump($res);

echo "Test 4\n";

$name = "";
$stmt = oci_parse($c, "UPDATE bind_test SET name=:name WHERE NAME = 'def'");
oci_bind_by_name($stmt, ":name", $name);

var_dump(oci_execute($stmt));

$stid = oci_parse($c, "select * from bind_test order by 1");
oci_execute($stid);
oci_fetch_all($stid, $res);
var_dump($res);


// Clean up

$drop = "DROP table bind_test";
$statement = oci_parse($c, $drop);
@oci_execute($statement);

echo "Done\n";

?>
--EXPECTF--
Test 1
bool(true)
Test 2
bool(true)
Test 3
bool(true)
array(1) {
  [u"NAME"]=>
  array(2) {
    [0]=>
    unicode(3) "def"
    [1]=>
    NULL
  }
}
Test 4
bool(true)
array(1) {
  [u"NAME"]=>
  array(2) {
    [0]=>
    NULL
    [1]=>
    NULL
  }
}
Done
