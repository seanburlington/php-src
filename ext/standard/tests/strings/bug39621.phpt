--TEST--
Bug #39621 (str_replace() is not binary safe on strings with equal length)
--FILE--
<?php

$search =  "qxxx\0qqqqqqqq";
$subject = "qxxx\0xxxxxxxx";
$replace = "any text";

$result = str_replace ( $search, $replace, $subject );

var_dump($result);

$search =  "QXXX\0qqqqqqqq";
$subject = "qxxx\0xxxxxxxx";
$replace = "any text";

$result = str_ireplace ( $search, $replace, $subject );

var_dump($result);

$search =  "qxxx\0xxxxxxxx";
$subject = "qxxx\0xxxxxxxx";
$replace = "any text";

$result = str_replace ( $search, $replace, $subject );

var_dump($result);

$search =  "qXxx\0xXxXxXxx";
$subject = "qxXx\0xxxxxxxx";
$replace = "any text";

$result = str_ireplace ( $search, $replace, $subject );

var_dump($result);

echo "Done\n";
?>
--EXPECT--
unicode(13) "qxxx xxxxxxxx"
unicode(13) "qxxx xxxxxxxx"
unicode(8) "any text"
unicode(8) "any text"
Done
