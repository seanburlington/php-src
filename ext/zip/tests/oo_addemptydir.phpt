--TEST--
ziparchive::addEmptyDir
--SKIPIF--
<?php
/* $Id: oo_addemptydir.phpt,v 1.1 2006/11/01 19:56:53 pajoye Exp $ */
if(!extension_loaded('zip')) die('skip');
?>
--FILE--
<?php

$dirname = dirname(__FILE__) . '/';
include $dirname . 'utils.inc';
$file = $dirname . '__tmp_oo_addfile.zip';

copy($dirname . 'test.zip', $file);

$zip = new ZipArchive;
if (!$zip->open($file)) {
	exit('failed');
}

$zip->addEmptyDir('emptydir');
if ($zip->status == ZIPARCHIVE::ER_OK) {
	dump_entries_name($zip);
	$zip->close();
} else {
	echo "failed\n";
}
@unlink($file);
?>
--EXPECTF--
0 bar
1 foobar/
2 foobar/baz
3 entry1.txt
4 emptydir/
