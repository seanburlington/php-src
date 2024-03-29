--TEST--
Bug # 28817: (properties in extended class)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

class z extends domDocument{
	/** variable can have name */
	public $p_array;
	public $p_variable;

	function __construct(){
		$this->p_array[] = 'bonus';
		$this->p_array[] = 'vir';
		$this->p_array[] = 'semper';
		$this->p_array[] = 'tiro';

		$this->p_variable = 'Cessante causa cessat effectus';
	}	
}

$z=new z();
var_dump($z->p_array);
var_dump($z->p_variable);
?>
--EXPECTF--
array(4) {
  [0]=>
  unicode(5) "bonus"
  [1]=>
  unicode(3) "vir"
  [2]=>
  unicode(6) "semper"
  [3]=>
  unicode(4) "tiro"
}
unicode(30) "Cessante causa cessat effectus"
