--TEST--
Bug #38698 (Bug #38698 for some keys cdbmake creates corrupted db and cdb can't read valid db)
--SKIPIF--
<?php 
	$handler = 'cdb_make';
	require_once('skipif.inc');
?>
--FILE--
<?php

$db_file = '129php.cdb';

if (($db_make=dba_open($db_file, "n", 'cdb_make'))!==FALSE) {
	dba_insert(pack('i',129), "Booo!", $db_make);
	dba_close($db_make);
	// write md5 checksum of generated database file
	var_dump(md5_file($db_file));
	@unlink($db_file);
} else {
    echo "Error creating database\n";
}
?>
===DONE===
--EXPECT--
unicode(32) "1f34b74bde3744265acfc21e0f30af95"
===DONE===
