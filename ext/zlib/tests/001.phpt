--TEST--
gzdeflate()/gzinflate()
--SKIPIF--
<?php if (!extension_loaded("zlib")) print "skip"; ?>
--FILE--
<?php /* $Id: 001.phpt,v 1.5 2006/07/27 16:31:07 nlopess Exp $ */
$original = str_repeat("hallo php",4096);
$packed=gzdeflate((binary)$original);
echo strlen($packed)." ".strlen($original)."\n";
$unpacked=gzinflate($packed);
if (strcmp($original,$unpacked)==0) echo "Strings are equal\n";

/* with explicit compression level, length */
$original = str_repeat("hallo php",4096);
$packed=gzdeflate((binary)$original, 9);
echo strlen($packed)." ".strlen($original)."\n";
$unpacked=gzinflate($packed, 40000);
if (strcmp($original,$unpacked)==0) echo "Strings are equal\n";

$original = b'aaaaaaaaaaaaaaa';
$packed=gzdeflate($original);
echo strlen($packed)." ".strlen($original)."\n";
$unpacked=gzinflate($packed);
if (strcmp($original,$unpacked)==0) echo "Strings are equal";
?>
--EXPECT--
100 36864
Strings are equal
100 36864
Strings are equal
5 15
Strings are equal