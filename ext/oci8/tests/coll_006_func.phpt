--TEST--
ocinewcollection() + free()
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php

require dirname(__FILE__)."/connect.inc";
require dirname(__FILE__)."/create_type.inc";

var_dump($coll1 = ocinewcollection($c, $type_name));

var_dump(oci_free_collection($coll1));
var_dump(oci_collection_size($coll1));

echo "Done\n";

require dirname(__FILE__)."/drop_type.inc";

?>
--EXPECTF--
object(OCI-Collection)#%d (1) {
  [u"collection"]=>
  resource(%d) of type (oci8 collection)
}
bool(true)

Warning: oci_collection_size(): %d is not a valid oci8 collection resource in %s on line %d
bool(false)
Done
