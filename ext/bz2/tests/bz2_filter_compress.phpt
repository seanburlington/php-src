--TEST--
bzip2.compress (with convert.base64-encode)
--SKIPIF--
<?php if (!extension_loaded("bz2")) print "skip"; ?>
--FILE--
<?php /* $Id: bz2_filter_compress.phpt,v 1.2 2006/04/02 17:41:04 pollita Exp $ */
$text = b'I am the very model of a modern major general, I\'ve information vegetable, animal, and mineral.';

$fp = fopen('php://stdout', 'w');
stream_filter_append($fp, 'bzip2.compress', STREAM_FILTER_WRITE);
stream_filter_append($fp, 'convert.base64-encode', STREAM_FILTER_WRITE);
fwrite($fp, $text);
fclose($fp);

?> 
--EXPECT-- 
QlpoNDFBWSZTWRN6QG0AAAoVgECFACA395UgIABIintI1N6mpowIQ0E1MTTAQGYTNcRyMZm5kgW3ib7hVboE7Tmqj3ToGZ5G3q1ZauD2G58hibSck8KS95EEAbx1Cn+LuSKcKEgJvSA2gA==
