--TEST--
Test fopen and fclose() functions - usage variations - "ab" mode 
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

/* Test fopen() and fclose(): Opening the file in "ab" mode,
   checking for the file creation, write & read operations,
   checking for the file pointer position,
   and fclose function
*/
$file_path = dirname(__FILE__);
require($file_path."/file.inc");

create_files($file_path, 1, "text_with_new_line", 0755, 20, "w", "007_variation", 21, "bytes");
$file = $file_path."/007_variation21.tmp";
$string = b"abcdefghij\nmnopqrst\tuvwxyz\n0123456789";

echo "*** Test fopen() & fclose() functions:  with 'ab' mode ***\n";
$file_handle = fopen($file, "ab");  //opening the file "ab" mode
var_dump($file_handle);  //Check for the content of handle
var_dump( get_resource_type($file_handle) );  //Check for the type of resource
var_dump( fwrite($file_handle, $string) );  //Check for write operation; passes; expected:size of the $string
rewind($file_handle);
var_dump( fread($file_handle, 100) );  //Check for read operation; fails; expected: empty string
var_dump( ftell($file_handle) );  //File pointer position after read operation, expected at the end of the file
var_dump( fclose($file_handle) );  //Check for close operation on the file handle
var_dump( get_resource_type($file_handle) );  //Check whether resource is lost after close operation
var_dump( filesize($file) ); //Check that data hasn't over written; Expected: Size of (initial data + newly added data)

unlink($file);  //Deleting the file
fclose( fopen($file, "ab") );  //Opening the non-existing file in "ab" mode, which will be created
var_dump( file_exists($file) );  //Check for the existance of file
echo "*** Done ***\n"; 
--CLEAN--
<?php
unlink(dirname(__FILE__)."/007_variation21.tmp");
?>
--EXPECTF--
*** Test fopen() & fclose() functions:  with 'ab' mode ***
resource(%d) of type (stream)
%unicode|string%(6) "stream"
int(37)
string(0) ""
int(0)
bool(true)
%unicode|string%(7) "Unknown"
int(57)
bool(true)
*** Done ***
