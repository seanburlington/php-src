--TEST--
Array fetch CLOB and BLOB
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php

require dirname(__FILE__).'/connect.inc';
require dirname(__FILE__).'/create_table.inc';

echo "Test 1: CLOB\n";

$ora_sql = "INSERT INTO
                       ".$schema.$table_name." (clob)
                      VALUES (empty_clob())
                      RETURNING
                               clob
                      INTO :v_clob ";

$s = oci_parse($c,$ora_sql);
$clob = oci_new_descriptor($c,OCI_DTYPE_LOB);


oci_bind_by_name($s,":v_clob", $clob,-1,OCI_B_CLOB);

oci_execute($s, OCI_DEFAULT);
var_dump($clob->save("clob test 1"));

oci_execute($s, OCI_DEFAULT);
var_dump($clob->save("clob test 2"));

oci_execute($s, OCI_DEFAULT);
var_dump($clob->save("clob test 3"));


$s = oci_parse($c,"select clob from ".$schema.$table_name);
var_dump(oci_execute($s));

oci_fetch_all($s, $res);

var_dump($res);


echo "Test 1b\n";

$s = oci_parse($c, "select clob from ".$schema.$table_name);
var_dump(oci_execute($s, OCI_DEFAULT));
while ($row = oci_fetch_array($s, OCI_ASSOC)) {
    var_dump($row);
    $result = $row['CLOB']->load();
    var_dump($result);
}


require dirname(__FILE__).'/drop_table.inc';

echo "Test 2: BLOB\n";

require dirname(__FILE__).'/create_table.inc';

$ora_sql = "INSERT INTO
                       ".$schema.$table_name." (blob)
                      VALUES (empty_blob())
                      RETURNING
                               blob
                      INTO :v_blob ";

$s = oci_parse($c,$ora_sql);
$blob = oci_new_descriptor($c,OCI_DTYPE_LOB);


oci_bind_by_name($s,":v_blob", $blob,-1,OCI_B_BLOB);

oci_execute($s, OCI_DEFAULT);
var_dump($blob->save("blob test 1"));

oci_execute($s, OCI_DEFAULT);
var_dump($blob->save("blob test 2"));

oci_execute($s, OCI_DEFAULT);
var_dump($blob->save("blob test 3"));

$s = oci_parse($c, "select blob from ".$schema.$table_name);
var_dump(oci_execute($s));
oci_fetch_all($s, $res);
var_dump($res);

echo "Test 2b\n";

$s = oci_parse($c, "select blob from ".$schema.$table_name);
var_dump(oci_execute($s, OCI_DEFAULT));
while ($row = oci_fetch_array($s, OCI_ASSOC)) {
    var_dump($row);
    $result = $row['BLOB']->load();
    var_dump($result);
}


require dirname(__FILE__).'/drop_table.inc';

echo "Done\n";

?>
--EXPECTF--
Test 1: CLOB
bool(true)
bool(true)
bool(true)
bool(true)
array(1) {
  [u"CLOB"]=>
  array(3) {
    [0]=>
    unicode(11) "clob test 1"
    [1]=>
    unicode(11) "clob test 2"
    [2]=>
    unicode(11) "clob test 3"
  }
}
Test 1b
bool(true)
array(1) {
  [u"CLOB"]=>
  object(OCI-Lob)#%d (1) {
    [u"descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
}
unicode(11) "clob test 1"
array(1) {
  [u"CLOB"]=>
  object(OCI-Lob)#%d (1) {
    [u"descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
}
unicode(11) "clob test 2"
array(1) {
  [u"CLOB"]=>
  object(OCI-Lob)#%d (1) {
    [u"descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
}
unicode(11) "clob test 3"
Test 2: BLOB
bool(true)
bool(true)
bool(true)
bool(true)
array(1) {
  [u"BLOB"]=>
  array(3) {
    [0]=>
    string(22) "b l o b   t e s t   1 "
    [1]=>
    string(22) "b l o b   t e s t   2 "
    [2]=>
    string(22) "b l o b   t e s t   3 "
  }
}
Test 2b
bool(true)
array(1) {
  [u"BLOB"]=>
  object(OCI-Lob)#%d (1) {
    [u"descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
}
string(22) "b l o b   t e s t   1 "
array(1) {
  [u"BLOB"]=>
  object(OCI-Lob)#%d (1) {
    [u"descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
}
string(22) "b l o b   t e s t   2 "
array(1) {
  [u"BLOB"]=>
  object(OCI-Lob)#%d (1) {
    [u"descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
}
string(22) "b l o b   t e s t   3 "
Done
