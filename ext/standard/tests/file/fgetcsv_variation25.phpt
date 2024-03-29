--TEST--
Test fgetcsv() : usage variations - with negative length value along with enclosure and delimiter
--FILE--
<?php
/* 
 Prototype: array fgetcsv ( resource $handle [, int $length [, string $delimiter [, string $enclosure]]] );
 Description: Gets line from file pointer and parse for CSV fields
*/

/* 
   Testing fgetcsv() to read from a file when provided with negative length argument 
   along with delimiter and enclosure arguments
*/
 
echo "*** Testing fgetcsv() : with negative length value ***\n";

/* the array is with three elements in it. Each element should be read as 
   1st element is delimiter, 2nd element is enclosure 
   and 3rd element is csv fields
*/
$csv_lists = array (
  array(',', '"', '"water",fruit'),
  array(',', '"', '"water","fruit"'),
  array(' ', '^', '^water^ ^fruit^'),
  array(':', '&', '&water&:&fruit&'),
  array('=', '=', '=water===fruit='),
  array('-', '-', '-water--fruit-air'),
  array('-', '-', '-water---fruit---air-'),
  array(':', '&', '&""""&:&"&:,:":&,&:,,,,')
);

$filename = dirname(__FILE__) . '/fgetcsv_variation25.tmp';
@unlink($filename);

$file_modes = array ("r","rb", "rt", "r+", "r+b", "r+t",
                     "a+", "a+b", "a+t",
                     "w+", "w+b", "w+t",
                     "x+", "x+b", "x+t"); 

$loop_counter = 1;
foreach ($csv_lists as $csv_list) {
  for($mode_counter = 0; $mode_counter < count($file_modes); $mode_counter++) {
    // create the file and add the content with has csv fields
    if ( strstr($file_modes[$mode_counter], "r") ) {
      $file_handle = fopen($filename, "w");
    } else {
      $file_handle = fopen($filename, $file_modes[$mode_counter] );
    }
    if ( !$file_handle ) {
      echo "Error: failed to create file $filename!\n";
      exit();
    }
    $delimiter = $csv_list[0];
    $enclosure = $csv_list[1];
    $csv_field = $csv_list[2];
    
    @fwrite($file_handle, $csv_field . "\n");
    // write another line of text and a blank line
    // this will be used to test, if the fgetcsv() read more than a line and its
    // working when only a blank line is read
    @fwrite($file_handle, "This is line of text without csv fields\n");
    @fwrite($file_handle, "\n"); // blank line

    // close the file if the mode to be used is read mode  and re-open using read mode
    // else rewind the file pointer to begining of the file 
    if ( strstr($file_modes[$mode_counter], "r" ) ) {
      fclose($file_handle);
      $file_handle = fopen($filename, $file_modes[$mode_counter]);
    } else {
      // rewind the file pointer to bof
      rewind($file_handle);
    }
      
    echo "\n-- Testing fgetcsv() with file opened using $file_modes[$mode_counter] mode --\n"; 

    // call fgetcsv() to parse csv fields
      
    // use the right delimiter and enclosure with negative length 
    var_dump( fgetcsv($file_handle, -10, $delimiter, $enclosure) );
    // check the file pointer position and if eof
    var_dump( ftell($file_handle) );
    var_dump( feof($file_handle) );
      
    // close the file
    fclose($file_handle);
    //delete file
    unlink($filename);
  } //end of mode loop 
} // end of foreach

echo "Done\n";
?>
--EXPECTF--
*** Testing fgetcsv() : with negative length value ***

-- Testing fgetcsv() with file opened using r mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rb mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rt mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rb mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rt mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rb mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rt mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rb mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rt mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rb mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rt mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rb mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rt mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rb mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rt mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rb mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using rt mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using r+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using a+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using w+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+ mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+b mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)

-- Testing fgetcsv() with file opened using x+t mode --

Warning: fgetcsv(): Length parameter may not be negative in %s on line %d
bool(false)
int(0)
bool(false)
Done
