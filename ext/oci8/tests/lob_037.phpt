--TEST--
Fetching two different lobs and using them after fetch
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php

require dirname(__FILE__).'/connect.inc';
require dirname(__FILE__).'/create_table.inc';

/* insert the first LOB */
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

var_dump($blob->write("first lob data"));
oci_commit($c);

/* insert the second LOB */
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

var_dump($blob->write("second lob data"));
oci_commit($c);

/* select both */

$ora_sql = "SELECT blob FROM ".$schema.$table_name;
$s = oci_parse($c,$ora_sql);
oci_execute($s, OCI_DEFAULT);

$rows = array();
$rows[0] = oci_fetch_assoc($s);
$rows[1] = oci_fetch_assoc($s);

var_dump($rows[0]['BLOB']->read(1000));
var_dump($rows[1]['BLOB']->read(1000));

require dirname(__FILE__).'/drop_table.inc';

echo "Done\n";

?>
--EXPECT--
int(28)
int(30)
string(28) "f i r s t   l o b   d a t a "
string(30) "s e c o n d   l o b   d a t a "
Done
