--TEST--
oci_fetch_all() - 2 
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php

require dirname(__FILE__)."/connect.inc";
require dirname(__FILE__).'/create_table.inc';

$insert_sql = "INSERT INTO ".$schema."".$table_name." (id, value) VALUES (1,1)";

$s = oci_parse($c, $insert_sql);

for ($i = 0; $i<3; $i++) {
	oci_execute($s);
}

oci_commit($c);

$select_sql = "SELECT * FROM ".$schema."".$table_name."";

$s = oci_parse($c, $select_sql);

oci_execute($s);
var_dump(oci_fetch_all($s, $all));
var_dump($all);

oci_execute($s);
var_dump(oci_fetch_all($s, $all, 0, 10, OCI_FETCHSTATEMENT_BY_ROW));
var_dump($all);

oci_execute($s);
var_dump(oci_fetch_all($s, $all, -1, -1, OCI_FETCHSTATEMENT_BY_ROW));
var_dump($all);

oci_execute($s);
var_dump(oci_fetch_all($s, $all, 0, 2, OCI_FETCHSTATEMENT_BY_ROW+OCI_NUM));
var_dump($all);

oci_execute($s);
var_dump(oci_fetch_all($s, $all, 0, 2, OCI_NUM));
var_dump($all);

oci_execute($s);
var_dump(oci_fetch_all($s, $all, 0, 1, OCI_BOTH));
var_dump($all);

require dirname(__FILE__).'/drop_table.inc';
	
echo "Done\n";
?>
--EXPECT--
int(3)
array(5) {
  [u"ID"]=>
  array(3) {
    [0]=>
    unicode(1) "1"
    [1]=>
    unicode(1) "1"
    [2]=>
    unicode(1) "1"
  }
  [u"VALUE"]=>
  array(3) {
    [0]=>
    unicode(1) "1"
    [1]=>
    unicode(1) "1"
    [2]=>
    unicode(1) "1"
  }
  [u"BLOB"]=>
  array(3) {
    [0]=>
    NULL
    [1]=>
    NULL
    [2]=>
    NULL
  }
  [u"CLOB"]=>
  array(3) {
    [0]=>
    NULL
    [1]=>
    NULL
    [2]=>
    NULL
  }
  [u"STRING"]=>
  array(3) {
    [0]=>
    NULL
    [1]=>
    NULL
    [2]=>
    NULL
  }
}
int(3)
array(3) {
  [0]=>
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
  [1]=>
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
  [2]=>
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
}
int(0)
array(0) {
}
int(2)
array(2) {
  [0]=>
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
  [1]=>
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
}
int(2)
array(5) {
  [0]=>
  array(2) {
    [0]=>
    unicode(1) "1"
    [1]=>
    unicode(1) "1"
  }
  [1]=>
  array(2) {
    [0]=>
    unicode(1) "1"
    [1]=>
    unicode(1) "1"
  }
  [2]=>
  array(2) {
    [0]=>
    NULL
    [1]=>
    NULL
  }
  [3]=>
  array(2) {
    [0]=>
    NULL
    [1]=>
    NULL
  }
  [4]=>
  array(2) {
    [0]=>
    NULL
    [1]=>
    NULL
  }
}
int(1)
array(5) {
  [0]=>
  array(1) {
    [0]=>
    unicode(1) "1"
  }
  [1]=>
  array(1) {
    [0]=>
    unicode(1) "1"
  }
  [2]=>
  array(1) {
    [0]=>
    NULL
  }
  [3]=>
  array(1) {
    [0]=>
    NULL
  }
  [4]=>
  array(1) {
    [0]=>
    NULL
  }
}
Done
