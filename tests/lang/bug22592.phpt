--TEST--
Bug #22592 (cascading assignments to strings with curly braces broken)
--FILE--
<?php
function error_hdlr($errno, $errstr) {
	echo "[$errstr]\n";
}

set_error_handler('error_hdlr');

$i = 4;
$s = "string";

$result = "* *-*";
var_dump($result);
$result{6} = '*';
var_dump($result);
$result{1} = $i;
var_dump($result);
$result{3} = $s;
var_dump($result);
$result{7} = 0;
var_dump($result);
$a = $result{1} = $result{3} = '-';
var_dump($result);
$b = $result{3} = $result{5} = $s;
var_dump($result);
$c = $result{0} = $result{2} = $result{4} = $i;
var_dump($result);
$d = $result{6} = $result{8} = 5;
var_dump($result);
$e = $result{1} = $result{6};
var_dump($result);
var_dump($a, $b, $c, $d, $e);
$result{-1} = 'a';
?>
--EXPECT--
unicode(5) "* *-*"
unicode(7) "* *-* *"
unicode(7) "*4*-* *"
unicode(7) "*4*s* *"
unicode(8) "*4*s* *0"
unicode(8) "*-*-* *0"
unicode(8) "*-*s*s*0"
unicode(8) "4-4s4s*0"
unicode(9) "4-4s4s505"
unicode(9) "454s4s505"
unicode(1) "-"
unicode(1) "s"
unicode(1) "4"
unicode(1) "5"
unicode(1) "5"
[Illegal string offset:  -1]
