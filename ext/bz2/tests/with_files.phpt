--TEST--
BZ2 with files
--SKIPIF--
<?php if (!extension_loaded("bz2")) print "skip"; ?>
--FILE--
<?php // $Id: with_files.phpt,v 1.3 2006/04/02 17:41:04 pollita Exp $

error_reporting(E_ALL);

$filename = "testfile.bz2";
$str = b"This is a test string.\n";
$bz = bzopen($filename, "w");
bzwrite($bz, $str);
bzclose($bz);

$bz = bzopen($filename, "r");
print bzread($bz, 10);
print bzread($bz);
bzclose($bz);
unlink($filename);

--EXPECT--
This is a test string.
