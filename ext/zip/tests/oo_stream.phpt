--TEST--
getStream
--SKIPIF--
<?php
/* $Id: oo_stream.phpt,v 1.4 2008/05/27 02:55:52 felipe Exp $ */
if(!extension_loaded('zip')) die('skip');
?>
--FILE--
<?php
$dirname = dirname(__FILE__) . '/';
$file = $dirname . 'test_with_comment.zip';
include $dirname . 'utils.inc';
$zip = new ZipArchive;
if (!$zip->open($file)) {
	exit('failed');
}
$fp = $zip->getStream('foo');

var_dump($fp);
if(!$fp) exit("\n");
$contents = b'';
while (!feof($fp)) {
	$contents .= fread($fp, 255);
}

fclose($fp);
$zip->close();
var_dump($contents);


$fp = fopen('zip://' . dirname(__FILE__) . '/test_with_comment.zip#foo', 'rb');
if (!$fp) {
  exit("cannot open\n");
}
$contents = b'';
while (!feof($fp)) {
  $contents .= fread($fp, 2);
}
var_dump($contents);
fclose($fp);

?>
--EXPECTF--
resource(%d) of type (stream)
unicode(5) "foo

"
string(5) "foo

"
