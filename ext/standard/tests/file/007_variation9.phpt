--TEST--
Test fopen and fclose() functions - usage variations - "rt" mode 
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

/* Test fopen() and fclose(): Opening the file in "rt" mode,
   checking for the file creation, write & read operations,
   checking for the file pointer position,
   and fclose function
*/
$file_path = dirname(__FILE__);
require($file_path."/file.inc");

create_files($file_path, 1, "text_with_new_line", 0755, 20, "w", "007_variation", 9, "bytes");
$file = $file_path."/007_variation9.tmp";
$string = "abcdefghij\nmnopqrst\tuvwxyz\n0123456789";

echo "*** Test fopen() & fclose() functions:  with 'rt' mode ***\n";
$file_handle = fopen($file, "rt");  //opening the file in "rt" mode
var_dump($file_handle);  //Check for the content of handle
var_dump( get_resource_type($file_handle) );  //Check for the type of resource
var_dump( ftell($file_handle) );  //Initial position of file pointer
var_dump( fread($file_handle, 100) );  //Check for read operation
var_dump( fwrite($file_handle, $string) );  //Check for write operation; fails; expected: 0 bytes
var_dump( fclose($file_handle) );  //Check for close operation on the file handle
var_dump( get_resource_type($file_handle) );  //Check whether resource is lost after close operation
echo "*** Done ***\n"; 
--CLEAN--
<?php
unlink(dirname(__FILE__)."/007_variation9.tmp");
?>
--EXPECTF--
*** Test fopen() & fclose() functions:  with 'rt' mode ***
resource(%d) of type (stream)
unicode(6) "stream"
int(0)
unicode(20) "line
line of text
li"

Notice: fwrite(): 37 character unicode buffer downcoded for binary stream runtime_encoding in %s on line %d
int(0)
bool(true)
unicode(7) "Unknown"
*** Done ***
