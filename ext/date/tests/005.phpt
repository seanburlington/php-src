--TEST--
idate() and invalid params
--INI--
date.timezone=UTC
--FILE--
<?php

$t = mktime(0,0,0, 6, 27, 2006);

var_dump(idate());
var_dump(idate(1,1,1));

var_dump(idate(1,1));
var_dump(idate(""));
var_dump(idate(0));

var_dump(idate("B", $t));
var_dump(idate("[", $t));
var_dump(idate("'"));

echo "Done\n";
?>
--EXPECTF--
Warning: idate() expects at least 1 parameter, 0 given in %s on line %d
bool(false)

Warning: idate() expects at most 2 parameters, 3 given in %s on line %d
bool(false)

Warning: idate(): Unrecognized date format token in %s on line %d
bool(false)

Warning: idate(): idate format is one char in %s on line %d
bool(false)

Warning: idate(): Unrecognized date format token in %s on line %d
bool(false)
int(41)

Warning: idate(): Unrecognized date format token in %s on line %d
bool(false)

Warning: idate(): Unrecognized date format token in %s on line %d
bool(false)
Done
