--TEST--
symlink() & friends
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == 'WIN') {
    die('skip no symlinks on Windows');
}
?>
--FILE--
<?php

$filename = dirname(__FILE__)."/symlink.dat";
$link = dirname(__FILE__)."/symlink.link";

var_dump(symlink($filename, $link));
var_dump(readlink($link));
var_dump(linkinfo($link));
@unlink($link);

var_dump(readlink($link));
var_dump(linkinfo($link));

touch($filename);
var_dump(symlink($filename, dirname(__FILE__)));
@unlink($link);

var_dump(symlink($filename, $link));
@unlink($link);

touch($link);
var_dump(symlink($filename, $link));
@unlink($link);

var_dump(link($filename, $link));
@unlink($filename);

var_dump(link($filename, $link));
@unlink($link);

var_dump(symlink(".", "."));
var_dump(link(".", "."));
var_dump(readlink("."));
var_dump(linkinfo("."));

echo "Done\n";
?>
--EXPECTF--
bool(true)
unicode(%d) "%ssymlink.dat"
int(%d)

Warning: readlink(): No such file or directory in %s on line %d
bool(false)

Warning: linkinfo(): No such file or directory in %s on line %d
int(-1)

Warning: symlink(): File exists in %s on line %d
bool(false)
bool(true)

Warning: symlink(): File exists in %s on line %d
bool(false)
bool(true)

Warning: link(): No such file or directory in %s on line %d
bool(false)

Warning: symlink(): %s in %s on line %d
bool(false)

Warning: link(): %s in %s on line %d
bool(false)

Warning: readlink(): Invalid argument in %s on line %d
bool(false)
int(%d)
Done
