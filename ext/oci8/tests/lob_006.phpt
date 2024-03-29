--TEST--
oci_lob_write()/truncate()/erase()
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

var_dump($blob);

$str = b"this is a biiiig faaat test string. why are you reading it, I wonder? =)";
var_dump($blob->write($str));
var_dump($blob->truncate());
var_dump($blob->seek(0, OCI_SEEK_SET));
var_dump($blob->write(b"string was here. tick-tack-tick-tack."));
var_dump($blob->erase(10, 10));
var_dump($blob->write(b"some"));

oci_commit($c);

$select_sql = "SELECT blob FROM ".$schema.$table_name." FOR UPDATE";
$s = oci_parse($c, $select_sql);
oci_execute($s, OCI_DEFAULT);

var_dump($row = oci_fetch_array($s));

var_dump($row[0]->read(10000));

require dirname(__FILE__).'/drop_table.inc';

echo "Done\n";

?>
--EXPECTF--
object(OCI-Lob)#%d (1) {
  [u"descriptor"]=>
  resource(%d) of type (oci8 descriptor)
}
int(72)
bool(true)
bool(true)
int(37)
int(10)
int(4)
array(2) {
  [0]=>
  object(OCI-Lob)#%d (1) {
    [u"descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
  [u"BLOB"]=>
  object(OCI-Lob)#%d (1) {
    [u"descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
}
string(41) "string was          k-tack-tick-tack.some"
Done
