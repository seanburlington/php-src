--TEST--
oci_lob_free()/close() 
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php
	
require dirname(__FILE__).'/connect.inc';
require dirname(__FILE__).'/create_table.inc';

$ora_sql = "INSERT INTO
                       ".$schema.$table_name." (blob)
                      VALUES (empty_blob())
                      RETURNING
                               blob
                      INTO :v_blob ";

$statement = oci_parse($c,$ora_sql);
$blob = oci_new_descriptor($c,OCI_D_LOB);
oci_bind_by_name($statement,":v_blob", $blob,-1,OCI_B_BLOB);
oci_execute($statement, OCI_DEFAULT);

var_dump($blob->write(b"test"));
var_dump($blob->close());
var_dump($blob->write(b"test"));
var_dump(oci_free_descriptor($blob));
var_dump($blob->write(b"test"));

var_dump(oci_free_descriptor($blob));
var_dump(oci_free_descriptor(new stdclass));

$blob = oci_new_descriptor($c,OCI_D_LOB);
unset($blob->descriptor);
var_dump(oci_free_descriptor($blob));

oci_commit($c);

$select_sql = "SELECT blob FROM ".$schema.$table_name."";
$s = oci_parse($c, $select_sql);
oci_execute($s);

var_dump(oci_fetch_array($s, OCI_NUM + OCI_RETURN_LOBS));

require dirname(__FILE__).'/drop_table.inc';

echo "Done\n";

?>
--EXPECTF--
int(4)
bool(true)
int(4)
bool(true)

Warning: OCI-Lob::write(): %d is not a valid oci8 descriptor resource in %s on line %d
bool(false)

Warning: oci_free_descriptor(): %d is not a valid oci8 descriptor resource in %s on line %d
bool(false)

Warning: oci_free_descriptor() expects parameter 1 to be OCI-Lob, object given in %s on line %d
NULL

Warning: oci_free_descriptor(): Unable to find descriptor property in %s on line %d
bool(false)
array(1) {
  [0]=>
  string(8) "testtest"
}
Done
