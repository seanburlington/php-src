--TEST--
Bug #40752 (parse_ini_file() segfaults when a scalar setting is redeclared as an array)
--FILE--
<?php

$file = dirname(__FILE__)."/bug40752.ini";
file_put_contents($file, '
foo   = 1;
foo[] = 1;
');

var_dump(parse_ini_file($file));

file_put_contents($file, '
foo[] = 1;
foo   = 1;
');

var_dump(parse_ini_file($file));

unlink($file);

echo "Done\n";
?>
--EXPECT--
array(1) {
  [u"foo"]=>
  array(1) {
    [0]=>
    unicode(1) "1"
  }
}
array(1) {
  [u"foo"]=>
  unicode(1) "1"
}
Done
