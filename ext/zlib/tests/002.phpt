--TEST--
gzcompress()/gzuncompress()
--SKIPIF--
<?php if (!extension_loaded("zlib")) print "skip"; ?>
--POST--
--GET--
--FILE--
<?php /* $Id: 002.phpt,v 1.2 2003/09/06 15:31:35 sr Exp $ */
$original = str_repeat("hallo php",4096);
$packed=gzcompress($original);
echo strlen($packed)." ".strlen($original)."\n";
$unpacked=gzuncompress($packed);
if (strcmp($original,$unpacked)==0) echo "Strings are equal\n";

/* with explicit compression level, length */
$original = str_repeat("hallo php",4096);
$packed=gzcompress($original, 9);
echo strlen($packed)." ".strlen($original)."\n";
$unpacked=gzuncompress($packed, 40000);
if (strcmp($original,$unpacked)==0) echo "Strings are equal\n";
?>
--EXPECT--
106 36864
Strings are equal
106 36864
Strings are equal
