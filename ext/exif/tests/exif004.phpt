--TEST--
Check for exif_read_data, Unicode WinXP tags	
--SKIPIF--
<?php 
	if (!extension_loaded('exif')) die('skip exif extension not available');
	if (!extension_loaded('mbstring')) die('skip mbstring extension not available');
	if (!defined("EXIF_USE_MBSTRING") || !EXIF_USE_MBSTRING) die ('skip mbstring loaded by dl');
?>
--INI--
output_handler=
zlib.output_compression=0
exif.decode_unicode_intel=UCS-2LE
exif.decode_unicode_motorola=UCS-2BE
exif.encode_unicode=ISO-8859-1
--FILE--
<?php
/*
  test4.jpg is a 1*1 image that contains Exif tags written by WindowsXP
*/
$image  = exif_read_data(dirname(__FILE__).'/test4.jpg','',true,false);
echo var_dump($image['WINXP']);
?>
--EXPECT--
array(5) {
  [u"Subject"]=>
  unicode(10) "Subject..."
  [u"Keywords"]=>
  unicode(11) "Keywords..."
  [u"Author"]=>
  unicode(9) "Rui Carmo"
  [u"Comments"]=>
  unicode(29) "Comments
Line2
Line3
Line4"
  [u"Title"]=>
  unicode(8) "Title..."
}
