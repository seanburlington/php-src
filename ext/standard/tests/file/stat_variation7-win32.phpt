--TEST--
Test stat() functions: usage variations - names of dir/file stored in objects
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) != 'WIN') {
    die('skip.. only for Windows');
}
?>
--FILE--
<?php

/*
 *  Prototype: array stat ( string $filename );
 *  Description: Gives information about a file
 */

/* test the stats of dir/file when their names are stored in objects */

$file_path = dirname(__FILE__);
require "$file_path/file.inc";


/* create temp file and directory */
mkdir("$file_path/stat_variation7/");  // temp dir

$file_handle = fopen("$file_path/stat_variation7.tmp", "w");  // temp file
fclose($file_handle);


echo "\n*** Testing stat(): with filename
    and directory name stored inside a object ***\n";

// creating object with members as numeric and non-numeric filename and directory name
class object_temp {
public $var_name;
public function object_temp($name) {
$this->var_name = $name;
  }
}

// directory as member
$obj1 = new object_temp("$file_path/stat_variation7/");
$obj2 = new object_temp("$file_path/stat_variation7a/");

// file as member
$obj3 = new object_temp("$file_path/stat_variation7.tmp");
$obj4 = new object_temp("$file_path/stat_variation7a.tmp");

echo "\n-- Testing stat() on filename stored inside an object --\n";
var_dump( stat($obj3->var_name) );

$file_handle = fopen("$file_path/stat_variation7a.tmp", "w");
fclose($file_handle);
var_dump( stat($obj4->var_name) );

echo "\n-- Testing stat() on directory name stored inside an object --\n";
var_dump( stat($obj1->var_name) );

mkdir("$file_path/stat_variation7a/");
var_dump( stat($obj2->var_name) );

echo "\n*** Done ***";
?>

--CLEAN--
<?php
$file_path = dirname(__FILE__);
unlink("$file_path/stat_variation7.tmp");
unlink("$file_path/stat_variation7a.tmp");
rmdir("$file_path/stat_variation7");
rmdir("$file_path/stat_variation7a");
?>
--EXPECTF--
*** Testing stat(): with filename
    and directory name stored inside a object ***

-- Testing stat() on filename stored inside an object --
array(26) {
  [0]=>
  int(%d)
  [1]=>
  int(%d)
  [2]=>
  int(%d)
  [3]=>
  int(%d)
  [4]=>
  int(%d)
  [5]=>
  int(%d)
  [6]=>
  int(%d)
  [7]=>
  int(%d)
  [8]=>
  int(%d)
  [9]=>
  int(%d)
  [10]=>
  int(%d)
  [11]=>
  int(-%d)
  [12]=>
  int(-%d)
  [u"dev"]=>
  int(%d)
  [u"ino"]=>
  int(%d)
  [u"mode"]=>
  int(%d)
  [u"nlink"]=>
  int(%d)
  [u"uid"]=>
  int(%d)
  [u"gid"]=>
  int(%d)
  [u"rdev"]=>
  int(%d)
  [u"size"]=>
  int(%d)
  [u"atime"]=>
  int(%d)
  [u"mtime"]=>
  int(%d)
  [u"ctime"]=>
  int(%d)
  [u"blksize"]=>
  int(-%d)
  [u"blocks"]=>
  int(-%d)
}
array(26) {
  [0]=>
  int(%d)
  [1]=>
  int(%d)
  [2]=>
  int(%d)
  [3]=>
  int(%d)
  [4]=>
  int(%d)
  [5]=>
  int(%d)
  [6]=>
  int(%d)
  [7]=>
  int(%d)
  [8]=>
  int(%d)
  [9]=>
  int(%d)
  [10]=>
  int(%d)
  [11]=>
  int(-%d)
  [12]=>
  int(-%d)
  [u"dev"]=>
  int(%d)
  [u"ino"]=>
  int(%d)
  [u"mode"]=>
  int(%d)
  [u"nlink"]=>
  int(%d)
  [u"uid"]=>
  int(%d)
  [u"gid"]=>
  int(%d)
  [u"rdev"]=>
  int(%d)
  [u"size"]=>
  int(%d)
  [u"atime"]=>
  int(%d)
  [u"mtime"]=>
  int(%d)
  [u"ctime"]=>
  int(%d)
  [u"blksize"]=>
  int(-%d)
  [u"blocks"]=>
  int(-%d)
}

-- Testing stat() on directory name stored inside an object --
array(26) {
  [0]=>
  int(%d)
  [1]=>
  int(%d)
  [2]=>
  int(%d)
  [3]=>
  int(%d)
  [4]=>
  int(%d)
  [5]=>
  int(%d)
  [6]=>
  int(%d)
  [7]=>
  int(%d)
  [8]=>
  int(%d)
  [9]=>
  int(%d)
  [10]=>
  int(%d)
  [11]=>
  int(-%d)
  [12]=>
  int(-%d)
  [u"dev"]=>
  int(%d)
  [u"ino"]=>
  int(%d)
  [u"mode"]=>
  int(%d)
  [u"nlink"]=>
  int(%d)
  [u"uid"]=>
  int(%d)
  [u"gid"]=>
  int(%d)
  [u"rdev"]=>
  int(%d)
  [u"size"]=>
  int(%d)
  [u"atime"]=>
  int(%d)
  [u"mtime"]=>
  int(%d)
  [u"ctime"]=>
  int(%d)
  [u"blksize"]=>
  int(-%d)
  [u"blocks"]=>
  int(-%d)
}
array(26) {
  [0]=>
  int(%d)
  [1]=>
  int(%d)
  [2]=>
  int(%d)
  [3]=>
  int(%d)
  [4]=>
  int(%d)
  [5]=>
  int(%d)
  [6]=>
  int(%d)
  [7]=>
  int(%d)
  [8]=>
  int(%d)
  [9]=>
  int(%d)
  [10]=>
  int(%d)
  [11]=>
  int(-%d)
  [12]=>
  int(-%d)
  [u"dev"]=>
  int(%d)
  [u"ino"]=>
  int(%d)
  [u"mode"]=>
  int(%d)
  [u"nlink"]=>
  int(%d)
  [u"uid"]=>
  int(%d)
  [u"gid"]=>
  int(%d)
  [u"rdev"]=>
  int(%d)
  [u"size"]=>
  int(%d)
  [u"atime"]=>
  int(%d)
  [u"mtime"]=>
  int(%d)
  [u"ctime"]=>
  int(%d)
  [u"blksize"]=>
  int(-%d)
  [u"blocks"]=>
  int(-%d)
}

*** Done ***
