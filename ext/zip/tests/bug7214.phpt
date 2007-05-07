--TEST--
Bug #7214 (zip_entry_read() binary safe)
--SKIPIF--
<?php
/* $Id: bug7214.phpt,v 1.2 2007/05/07 16:50:39 tony2001 Exp $ */
if(!extension_loaded('zip')) die('skip');
 ?>
--FILE--
<?php
$zip = zip_open(dirname(__FILE__)."/binarynull.zip");
if (!is_resource($zip)) die("Failure");
$entries = 0;
$entry = zip_read($zip);
$contents = zip_entry_read($entry, zip_entry_filesize($entry));
if (strlen($contents) == zip_entry_filesize($entry)) {
	echo "Ok";
} else {
	echo "failed";
}

?>
--EXPECT--
Ok
