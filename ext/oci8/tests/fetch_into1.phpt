--TEST--
various ocifetchinto() tests
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php

require dirname(__FILE__)."/connect.inc";
require dirname(__FILE__).'/create_table.inc';

$insert_sql = "INSERT INTO ".$schema."".$table_name." (id, value, string) VALUES (1, 1, NULL)";

if (!($s = oci_parse($c, $insert_sql))) {
	die("oci_parse(insert) failed!\n");
}

for ($i = 0; $i<20; $i++) {
	if (!oci_execute($s)) {
		die("oci_execute(insert) failed!\n");
	}
}

if (!oci_commit($c)) {
	die("oci_commit() failed!\n");
}

$select_sql = "SELECT * FROM ".$schema."".$table_name."";

if (!($s = oci_parse($c, $select_sql))) {
	die("oci_parse(select) failed!\n");
}

if (!oci_execute($s)) {
	die("oci_execute(select) failed!\n");
}
var_dump(ocifetchinto($s, $all, OCI_NUM));
var_dump($all);
var_dump(ocifetchinto($s, $all, OCI_ASSOC));
var_dump($all);
var_dump(ocifetchinto($s, $all, OCI_RETURN_NULLS));
var_dump($all);
var_dump(ocifetchinto($s, $all, OCI_RETURN_LOBS));
var_dump($all);
var_dump(ocifetchinto($s, $all, OCI_NUM+OCI_ASSOC));
var_dump($all);
var_dump(ocifetchinto($s, $all, OCI_NUM+OCI_ASSOC+OCI_RETURN_NULLS));
var_dump($all);
var_dump(ocifetchinto($s, $all, OCI_NUM+OCI_ASSOC+OCI_RETURN_NULLS+OCI_RETURN_LOBS));
var_dump($all);
var_dump(ocifetchinto($s, $all, OCI_RETURN_NULLS+OCI_RETURN_LOBS));
var_dump($all);
var_dump(ocifetchinto($s, $all, OCI_ASSOC+OCI_RETURN_NULLS+OCI_RETURN_LOBS));
var_dump($all);
var_dump(ocifetchinto($s, $all, OCI_NUM+OCI_RETURN_NULLS+OCI_RETURN_LOBS));
var_dump($all);

require dirname(__FILE__).'/drop_table.inc';
	
echo "Done\n";
?>
--EXPECT--
int(5)
array(2) {
  [0]=>
  unicode(1) "1"
  [1]=>
  unicode(1) "1"
}
int(5)
array(2) {
  [u"ID"]=>
  unicode(1) "1"
  [u"VALUE"]=>
  unicode(1) "1"
}
int(5)
array(5) {
  [0]=>
  unicode(1) "1"
  [1]=>
  unicode(1) "1"
  [2]=>
  NULL
  [3]=>
  NULL
  [4]=>
  NULL
}
int(5)
array(2) {
  [0]=>
  unicode(1) "1"
  [1]=>
  unicode(1) "1"
}
int(5)
array(4) {
  [0]=>
  unicode(1) "1"
  [u"ID"]=>
  unicode(1) "1"
  [1]=>
  unicode(1) "1"
  [u"VALUE"]=>
  unicode(1) "1"
}
int(5)
array(10) {
  [0]=>
  unicode(1) "1"
  [u"ID"]=>
  unicode(1) "1"
  [1]=>
  unicode(1) "1"
  [u"VALUE"]=>
  unicode(1) "1"
  [2]=>
  NULL
  [u"BLOB"]=>
  NULL
  [3]=>
  NULL
  [u"CLOB"]=>
  NULL
  [4]=>
  NULL
  [u"STRING"]=>
  NULL
}
int(5)
array(10) {
  [0]=>
  unicode(1) "1"
  [u"ID"]=>
  unicode(1) "1"
  [1]=>
  unicode(1) "1"
  [u"VALUE"]=>
  unicode(1) "1"
  [2]=>
  NULL
  [u"BLOB"]=>
  NULL
  [3]=>
  NULL
  [u"CLOB"]=>
  NULL
  [4]=>
  NULL
  [u"STRING"]=>
  NULL
}
int(5)
array(5) {
  [0]=>
  unicode(1) "1"
  [1]=>
  unicode(1) "1"
  [2]=>
  NULL
  [3]=>
  NULL
  [4]=>
  NULL
}
int(5)
array(5) {
  [u"ID"]=>
  unicode(1) "1"
  [u"VALUE"]=>
  unicode(1) "1"
  [u"BLOB"]=>
  NULL
  [u"CLOB"]=>
  NULL
  [u"STRING"]=>
  NULL
}
int(5)
array(5) {
  [0]=>
  unicode(1) "1"
  [1]=>
  unicode(1) "1"
  [2]=>
  NULL
  [3]=>
  NULL
  [4]=>
  NULL
}
Done
