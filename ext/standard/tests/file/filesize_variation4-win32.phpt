--TEST--
Test filesize() function: usage variations - file mode & holes in file
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) != 'WIN') {
    die('skip only valid for Windows');
}
--FILE--
<?php
/* 
 Prototype   : int filesize ( string $filename );
 Description : Returns the size of the file in bytes, or FALSE 
   (and generates an error of level E_WARNING) in case of an error.
*/

$file_path = dirname(__FILE__);
require($file_path."/file.inc");

echo "*** Testing filesize(): usage variations ***\n"; 
echo "\n*** Testing filesize() with data written using different file modes and by creating holes in file ***\n";

$filename = $file_path."/filesize_variation4.tmp";
$string = "Test 2 test the filesize() fn, with data containing all the types like !@@##$%^&*():<>?|~+!;',.\][{}(special) chars, 12345(numeric) chars, and \n(newline char), \t(tab), \0, \r and so on........\0";
echo "-- opening the file in 'w' mode and get the size --\n";
$file_handle = fopen($filename, "w");
var_dump( strlen($string) );  //strlen of the string
fwrite($file_handle, $string);
fclose($file_handle);
var_dump( filesize($filename) );  //size of the file = strlen of string
clearstatcache();

echo "-- opening the file in 'wt' mode and get the size --\n";
$file_handle = fopen($filename, "wt");
var_dump( strlen($string) );  //strlen of the string = 191 bytes
fwrite($file_handle, $string);
fclose($file_handle);
var_dump( filesize($filename) );  //size of the file = 192 bytes != strlen of string
clearstatcache();

echo "-- opening the file in 'a' mode, adding data and checking the file --\n";
$file_handle = fopen($filename, "a");
fwrite($file_handle, "Hello, world");
fclose($file_handle);
var_dump( filesize($filename) );  //204 bytes
clearstatcache();

echo "-- opening the file in 'at' mode, adding data and checking the file --\n";
$file_handle = fopen($filename, "at");
fwrite($file_handle, "Hello, world\n");
fclose($file_handle);
var_dump( filesize($filename) );  //218 bytes
clearstatcache();

echo "-- creating a hole and checking the size --\n";
$file_handle = fopen($filename, "a");
var_dump( ftruncate($file_handle, 220) );  //creating 4 bytes of hole
fclose($file_handle);
var_dump( filesize($filename) );  //220 bytes
clearstatcache();

echo "-- writing data after hole and checking the size --\n";
$file_handle = fopen($filename, "a");
fwrite($file_handle, "Hello\0");  //wrting 6 bytes of data
fclose($file_handle);
var_dump( filesize($filename) );  //226 bytes
clearstatcache();

echo "-- opening the existing file in write mode --\n";
fclose( fopen($filename, "w") );
var_dump( filesize($filename) );  //0 bytes
clearstatcache();

echo "-- with empty file --\n";
$filename = dirname(__FILE__)."/filesize_variation4_empty.tmp";
fclose( fopen($filename, "w") );
var_dump( filesize($filename) );  //0 bytes

echo "*** Done ***\n";
?>
--CLEAN--
<?php
$file_path = dirname(__FILE__);
unlink($file_path."/filesize_variation4.tmp");
unlink($file_path."/filesize_variation4_empty.tmp");
?>
--EXPECTF--
*** Testing filesize(): usage variations ***

*** Testing filesize() with data written using different file modes and by creating holes in file ***
-- opening the file in 'w' mode and get the size --
int(191)

Notice: fwrite(): 191 character unicode buffer downcoded for binary stream runtime_encoding in %s on line %d
int(191)
-- opening the file in 'wt' mode and get the size --
int(191)
int(192)
-- opening the file in 'a' mode, adding data and checking the file --

Notice: fwrite(): 12 character unicode buffer downcoded for binary stream runtime_encoding in %s on line %d
int(204)
-- opening the file in 'at' mode, adding data and checking the file --
int(218)
-- creating a hole and checking the size --
bool(true)
int(220)
-- writing data after hole and checking the size --

Notice: fwrite(): 6 character unicode buffer downcoded for binary stream runtime_encoding in %s on line %d
int(226)
-- opening the existing file in write mode --
int(0)
-- with empty file --
int(0)
*** Done ***
