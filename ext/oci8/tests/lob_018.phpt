--TEST--
fetching the same lob several times
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php
	
require dirname(__FILE__).'/connect.inc';

$drop = "DROP table lob_test";
$statement = oci_parse($c, $drop);
@oci_execute($statement);

$create = "CREATE table lob_test(mykey NUMBER, lob_1 CLOB)";
$statement = oci_parse($c, $create);
oci_execute($statement);

$init = "INSERT INTO lob_test (mykey, lob_1) VALUES(1, EMPTY_CLOB()) RETURNING lob_1 INTO :mylob";
$statement = oci_parse($c, $init);
$clob = oci_new_descriptor($c, OCI_D_LOB);
oci_bind_by_name($statement, ":mylob", $clob, -1, OCI_B_CLOB);
oci_execute($statement, OCI_DEFAULT);
$clob->save("data");

oci_commit($c);

$init = "INSERT INTO lob_test (mykey, lob_1) VALUES(2, EMPTY_CLOB()) RETURNING lob_1 INTO :mylob";
$statement = oci_parse($c, $init);
$clob = oci_new_descriptor($c, OCI_D_LOB);
oci_bind_by_name($statement, ":mylob", $clob, -1, OCI_B_CLOB);
oci_execute($statement, OCI_DEFAULT);
$clob->save("long data");

oci_commit($c);


$query = 'SELECT * FROM lob_test ORDER BY mykey ASC';
$statement = oci_parse ($c, $query);
oci_execute($statement, OCI_DEFAULT);

while ($row = oci_fetch_array($statement, OCI_ASSOC)) {
	$result = $row['LOB_1']->load();
	var_dump($result);
}

$query = 'SELECT * FROM lob_test ORDER BY mykey DESC';
$statement = oci_parse ($c, $query);
oci_execute($statement, OCI_DEFAULT);

while ($row = oci_fetch_array($statement, OCI_ASSOC)) {
	$result = $row['LOB_1']->load();
	var_dump($result);
}

$drop = "DROP table lob_test";
$statement = oci_parse($c, $drop);
@oci_execute($statement);

echo "Done\n";

?>
--EXPECTF--
unicode(4) "data"
unicode(9) "long data"
unicode(9) "long data"
unicode(4) "data"
Done
