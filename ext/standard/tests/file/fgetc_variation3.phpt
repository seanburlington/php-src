--TEST--
Test fgetc() function : usage variations - write only modes (Bug #42036)
--FILE--
<?php
/*
 Prototype: string fgetc ( resource $handle );
 Description: Gets character from file pointer
*/

/* try fgetc on files which are opened in non readable modes
    w, wb, wt,
    a, ab, at,
    x, xb, xt
*/
// include the header for common test function 
include ("file.inc");

echo "*** Testing fgetc() with file opened in write only mode ***\n";

$file_modes = array("w", "wb", "wt", "a", "ab", "at", "x", "xb", "xt");
$filename = dirname(__FILE__)."/fgetc_variation3.tmp";
foreach ($file_modes as $file_mode ) {
  echo "-- File opened in mode : $file_mode --\n";

  $file_handle = fopen($filename, $file_mode);
  if(!$file_handle) {
    echo "Error: failed to open file $filename!\n";
    exit();
  }
  $data = "fgetc_variation test";
  fwrite($file_handle, $data);

  // rewind the file pointer to begining of the file
  var_dump( rewind($file_handle) ); 
  var_dump( ftell($file_handle) ); 
  var_dump( feof($file_handle) );

  // read from file
  var_dump( fgetc($file_handle) ); // expected : no chars should be read
  var_dump( ftell($file_handle) ); // ensure that file pointer position is not changed
  var_dump( feof($file_handle) ); // check if end of file pointer is set

  // close the file
  fclose($file_handle);

  // delete the file
  unlink($filename); 
}

echo "Done\n";
?>
--EXPECTF--
*** Testing fgetc() with file opened in write only mode ***
-- File opened in mode : w --

Notice: fwrite(): 20 character unicode buffer downcoded for binary stream runtime_encoding in %sfgetc_variation3.php on line %d
bool(true)
int(0)
bool(false)
bool(false)
int(0)
bool(false)
-- File opened in mode : wb --

Notice: fwrite(): 20 character unicode buffer downcoded for binary stream runtime_encoding in %sfgetc_variation3.php on line %d
bool(true)
int(0)
bool(false)
bool(false)
int(0)
bool(false)
-- File opened in mode : wt --
bool(true)
int(0)
bool(false)
bool(false)
int(0)
bool(false)
-- File opened in mode : a --

Notice: fwrite(): 20 character unicode buffer downcoded for binary stream runtime_encoding in %sfgetc_variation3.php on line %d
bool(true)
int(0)
bool(false)
bool(false)
int(0)
bool(false)
-- File opened in mode : ab --

Notice: fwrite(): 20 character unicode buffer downcoded for binary stream runtime_encoding in %sfgetc_variation3.php on line %d
bool(true)
int(0)
bool(false)
bool(false)
int(0)
bool(false)
-- File opened in mode : at --
bool(true)
int(0)
bool(false)
bool(false)
int(0)
bool(false)
-- File opened in mode : x --

Notice: fwrite(): 20 character unicode buffer downcoded for binary stream runtime_encoding in %sfgetc_variation3.php on line %d
bool(true)
int(0)
bool(false)
bool(false)
int(0)
bool(false)
-- File opened in mode : xb --

Notice: fwrite(): 20 character unicode buffer downcoded for binary stream runtime_encoding in %sfgetc_variation3.php on line %d
bool(true)
int(0)
bool(false)
bool(false)
int(0)
bool(false)
-- File opened in mode : xt --

Notice: fwrite(): 20 character unicode buffer downcoded for binary stream runtime_encoding in %sfgetc_variation3.php on line %d
bool(true)
int(0)
bool(false)
bool(false)
int(0)
bool(false)
Done
