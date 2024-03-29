--TEST--
Check oci_pconnect try/catch end-of-scope with old_oci_close_semantics Off
--SKIPIF--
<?php if (!extension_loaded('oci8')) die ("skip no oci8 extension"); ?>
--INI--
oci8.old_oci_close_semantics=0
--FILE--
<?php

require(dirname(__FILE__).'/details.inc');

// Initialization

$stmtarray = array(
	"drop table scope_try5_tab",
	"create table scope_try5_tab (c1 number)"
);

if (!empty($dbase))
	$c1 = oci_new_connect($user,$password,$dbase);
else
	$c1 = oci_new_connect($user,$password);
						 
foreach ($stmtarray as $stmt) {
	$s1 = oci_parse($c1, $stmt);
	@oci_execute($s1);
}

// Run Test

echo "Test 1\n";

// Make errors throw exceptions

set_error_handler(create_function('$x, $y', 'throw new Exception($y, $x);'));

try
{
	if (!empty($dbase))
		$c = oci_pconnect($user,$password,$dbase);
	else
		$c = oci_pconnect($user,$password);
	$s = oci_parse($c, "insert into scope_try5_tab values (1)");
	oci_execute($s, OCI_DEFAULT);  // no commit
	$s = oci_parse($c, "insert into scope_try5_tab values (ABC)"); // syntax error -> throws exception
	oci_execute($s, OCI_DEFAULT);  // no commit
}
catch (Exception $e)
{
	echo "Caught Exception: ". $e->getMessage(), "\n";
	var_dump($c);

	// Verify data is not yet committed
	$s1 = oci_parse($c1, "select * from scope_try5_tab");
	oci_execute($s1);
	oci_fetch_all($s1, $r);
	var_dump($r);

	// Now commit
	oci_commit($c);
}

// Verify data was committed in the Catch block

$s1 = oci_parse($c1, "select * from scope_try5_tab");
oci_execute($s1);
oci_fetch_all($s1, $r);
var_dump($r);

// Cleanup

$stmtarray = array(
	"drop table scope_try5_tab"
);

foreach ($stmtarray as $stmt) {
	$s1 = oci_parse($c1, $stmt);
	oci_execute($s1);
}

echo "Done\n";

?>
--EXPECTF--
Test 1
Caught Exception: oci_execute(): ORA-00984: %s
resource(%d) of type (oci8 persistent connection)
array(1) {
  [u"C1"]=>
  array(0) {
  }
}
array(1) {
  [u"C1"]=>
  array(1) {
    [0]=>
    unicode(1) "1"
  }
}
Done
