--TEST--
Test fopen and fclose() functions - usage variations - "w+" mode 
--FILE--
<?php
/*
 fopen() function:
 Prototype: resource fopen(string $filename, string $mode
                            [, bool $use_include_path [, resource $context]] );
 Description: Opens file or URL.
*/
/*
 fclose() function:
 Prototype: bool fclose ( resource $handle );
 Description: Closes an open file pointer
*/

/* Test fopen() and fclose(): Opening the file in "w+" mode,
   checking for the file creation, write & read operations,
   checking for the file pointer position,
   checking for the file truncation when trying to open an existing file in "w+" mode,
   and fclose function
*/
$file_path = dirname(__FILE__);
require($file_path."/file.inc");

create_files($file_path, 1, "text_with_new_line", 0755, 20, "w", "007_variation", 4, "bytes");
$file = $file_path."/007_variation4.tmp";
$string = "abcdefghij\nmnopqrst\tuvwxyz\n0123456789";

echo "*** Test fopen() & fclose() functions:  with 'w+' mode ***\n";
$file_handle = fopen($file, "w+");  //opening the file "w+" mode
var_dump($file_handle);  //Check for the content of handle
var_dump( get_resource_type($file_handle) );  //Check for the type of resource
var_dump( ftell($file_handle) );  //Initial file pointer position, expected at the begining of the file
var_dump( fwrite($file_handle, $string) );  //Check for write operation; passes; expected:size of the $string
var_dump( ftell($file_handle) );  //File pointer position after write operation, expected at the end of the file
rewind($file_handle);
var_dump( fread($file_handle, 100) );  //Check for read operation; passes; expected: content of file
var_dump( ftell($file_handle) );  //File pointer position after read operation, expected at the end of the file
var_dump( fclose($file_handle) );  //Check for close operation on the file handle
var_dump( get_resource_type($file_handle) );  //Check whether resource is lost after close operation

var_dump( filesize($file) );  //Check for size of existing data file before opening the file in "w+" mode again, expected: size of content
clearstatcache();
fclose( fopen($file, "w+") );  //Opening the existing data file again in "w+" mode
var_dump( filesize($file) );  //Check for size of existing data file after opening the file in "w+" mode again, expected: 0 bytes
clearstatcache();

unlink($file);  //Deleting the file
fclose( fopen($file, "w+") );  //Opening the non-existing file in "w+" mode, which will be created
var_dump( file_exists($file) );  //Check for the existance of file
echo "*** Done ***\n"; 
--CLEAN--
<?php
unlink(dirname(__FILE__)."/007_variation4.tmp");
?>
--EXPECTF--
*** Test fopen() & fclose() functions:  with 'w+' mode ***
resource(%d) of type (stream)
unicode(6) "stream"
int(0)

Notice: fwrite(): 37 character unicode buffer downcoded for binary stream runtime_encoding in %s on line %d
int(37)
int(37)
string(37) "abcdefghij
mnopqrst	uvwxyz
0123456789"
int(37)
bool(true)
unicode(7) "Unknown"
int(37)
int(0)
bool(true)
*** Done ***
