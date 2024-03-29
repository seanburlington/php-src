--TEST--
oci_commit()/oci_rollback()
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php

require dirname(__FILE__)."/connect.inc";
require dirname(__FILE__).'/create_table.inc';

$insert_sql = "INSERT INTO ".$schema.$table_name." (id, value) VALUES (1,1)";

if (!($s = oci_parse($c, $insert_sql))) {
	die("oci_parse(insert) failed!\n");
}

for ($i = 0; $i<3; $i++) {
	if (!oci_execute($s, OCI_DEFAULT)) {
		die("oci_execute(insert) failed!\n");
	}
}

var_dump(oci_rollback($c));

$select_sql = "SELECT * FROM ".$schema.$table_name."";

if (!($select = oci_parse($c, $select_sql))) {
	die("oci_parse(select) failed!\n");
}

/* oci_fetch_all */
if (!oci_execute($select)) {
	die("oci_execute(select) failed!\n");
}
var_dump(oci_fetch_all($select, $all));
var_dump($all);

/* ocifetchstatement */
if (!oci_execute($s)) {
	die("oci_execute(select) failed!\n");
}

$insert_sql = "INSERT INTO ".$schema.$table_name." (id, value) VALUES (1,1)";

if (!($s = oci_parse($c, $insert_sql))) {
    die("oci_parse(insert) failed!\n");
}

for ($i = 0; $i<3; $i++) {
    if (!oci_execute($s, OCI_DEFAULT)) {
        die("oci_execute(insert) failed!\n");
    }
}

var_dump(oci_commit($c));

/* oci_fetch_all */
if (!oci_execute($select)) {
	die("oci_execute(select) failed!\n");
}
var_dump(oci_fetch_all($select, $all));
var_dump($all);


require dirname(__FILE__).'/drop_table.inc';
	
echo "Done\n";
?>
--EXPECT--
bool(true)
int(0)
array(5) {
  [u"ID"]=>
  array(0) {
  }
  [u"VALUE"]=>
  array(0) {
  }
  [u"BLOB"]=>
  array(0) {
  }
  [u"CLOB"]=>
  array(0) {
  }
  [u"STRING"]=>
  array(0) {
  }
}
bool(true)
int(4)
array(5) {
  [u"ID"]=>
  array(4) {
    [0]=>
    unicode(1) "1"
    [1]=>
    unicode(1) "1"
    [2]=>
    unicode(1) "1"
    [3]=>
    unicode(1) "1"
  }
  [u"VALUE"]=>
  array(4) {
    [0]=>
    unicode(1) "1"
    [1]=>
    unicode(1) "1"
    [2]=>
    unicode(1) "1"
    [3]=>
    unicode(1) "1"
  }
  [u"BLOB"]=>
  array(4) {
    [0]=>
    NULL
    [1]=>
    NULL
    [2]=>
    NULL
    [3]=>
    NULL
  }
  [u"CLOB"]=>
  array(4) {
    [0]=>
    NULL
    [1]=>
    NULL
    [2]=>
    NULL
    [3]=>
    NULL
  }
  [u"STRING"]=>
  array(4) {
    [0]=>
    NULL
    [1]=>
    NULL
    [2]=>
    NULL
    [3]=>
    NULL
  }
}
Done
