--TEST--
Test fopen and fclose() functions - usage variations - "a+b" mode 
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

/* Test fopen() and fclose(): Opening the file in "a+b" mode,
   checking for the file creation, write & read operations,
   checking for the file pointer position,
   and fclose function
*/
$file_path = dirname(__FILE__);
require($file_path."/file.inc");

create_files($file_path, 1, "text_with_new_line", 0755, 20, "w", "007_variation", 22, "bytes");
$file = $file_path."/007_variation22.tmp";
$string = "abcdefghij\nmnopqrst\tuvwxyz\n0123456789";

echo "*** Test fopen() & fclose() functions:  with 'a+b' mode ***\n";
$file_handle = fopen($file, "a+b");  //opening the file "a+b" mode
var_dump($file_handle);  //Check for the content of handle
var_dump( get_resource_type($file_handle) );  //Check for the type of resource
var_dump( fwrite($file_handle, $string) );  //Check for write operation; passes; expected:size of the $string
rewind($file_handle);
var_dump( fread($file_handle, 100) );  //Check for read operation; passes; expected: content of file
var_dump( ftell($file_handle) );  //File pointer position after read operation, expected at the end of the file
var_dump( fclose($file_handle) );  //Check for close operation on the file handle
var_dump( get_resource_type($file_handle) );  //Check whether resource is lost after close operation

unlink($file);  //Deleting the file
fclose( fopen($file, "a+b") );  //Opening the non-existing file in "a+b" mode, which will be created
var_dump( file_exists($file) );  //Check for the existance of file
echo "*** Done ***\n"; 
--CLEAN--
<?php
unlink(dirname(__FILE__)."/007_variation22.tmp");
?>
--EXPECTF--
*** Test fopen() & fclose() functions:  with 'a+b' mode ***
resource(%d) of type (stream)
unicode(6) "stream"

Notice: fwrite(): 37 character unicode buffer downcoded for binary stream runtime_encoding in %s on line %d
int(37)
string(57) "line
line of text
liabcdefghij
mnopqrst	uvwxyz
0123456789"
int(57)
bool(true)
unicode(7) "Unknown"
bool(true)
*** Done ***
