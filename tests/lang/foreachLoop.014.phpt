--TEST--
Directly modifying a REFERENCED array when foreach'ing over it.
--FILE--
<?php

define('MAX_LOOPS',5);

function withRefValue($elements, $transform) {
	echo "\n---( Array with $elements element(s): )---\n";
	//Build array:
	for ($i=0; $i<$elements; $i++) {
		$a[] = "v.$i";
	}
	$counter=0;
	
	$ref = &$a;
	
	echo "--> State of referenced array before loop:\n";
	var_dump($a);
	
	echo "--> Do loop:\n";	
	foreach ($a as $k=>$v) {
		echo "     iteration $counter:  \$k=$k; \$v=$v\n";
		eval($transform);
		$counter++;
		if ($counter>MAX_LOOPS) {
			echo "  ** Stuck in a loop! **\n";
			break;
		}
	}
	
	echo "--> State of array after loop:\n";
	var_dump($a);
}


echo "\nPopping elements off end of a referenced array";
$transform = 'array_pop($a);';
withRefValue(1, $transform);
withRefValue(2, $transform);
withRefValue(3, $transform);
withRefValue(4, $transform);

echo "\n\n\nShift elements off start of a referenced array";
$transform = 'array_shift($a);';
withRefValue(1, $transform);
withRefValue(2, $transform);
withRefValue(3, $transform);
withRefValue(4, $transform);

echo "\n\n\nRemove current element of a referenced array";
$transform = 'unset($a[$k]);';
withRefValue(1, $transform);
withRefValue(2, $transform);
withRefValue(3, $transform);
withRefValue(4, $transform);

echo "\n\n\nAdding elements to the end of a referenced array";
$transform = 'array_push($a, "new.$counter");';
withRefValue(1, $transform);
withRefValue(2, $transform);
withRefValue(3, $transform);
withRefValue(4, $transform);

echo "\n\n\nAdding elements to the start of a referenced array";
$transform = 'array_unshift($a, "new.$counter");';
withRefValue(1, $transform);
withRefValue(2, $transform);
withRefValue(3, $transform);
withRefValue(4, $transform);

?>
--EXPECTF--
Popping elements off end of a referenced array
---( Array with 1 element(s): )---
--> State of referenced array before loop:
array(1) {
  [0]=>
  unicode(3) "v.0"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
--> State of array after loop:
array(0) {
}

---( Array with 2 element(s): )---
--> State of referenced array before loop:
array(2) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=0; $v=v.0
--> State of array after loop:
array(0) {
}

---( Array with 3 element(s): )---
--> State of referenced array before loop:
array(3) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=1; $v=v.1
--> State of array after loop:
array(1) {
  [0]=>
  unicode(3) "v.0"
}

---( Array with 4 element(s): )---
--> State of referenced array before loop:
array(4) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
  [3]=>
  unicode(3) "v.3"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=1; $v=v.1
     iteration 2:  $k=0; $v=v.0
     iteration 3:  $k=0; $v=v.0
--> State of array after loop:
array(0) {
}



Shift elements off start of a referenced array
---( Array with 1 element(s): )---
--> State of referenced array before loop:
array(1) {
  [0]=>
  unicode(3) "v.0"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
--> State of array after loop:
array(0) {
}

---( Array with 2 element(s): )---
--> State of referenced array before loop:
array(2) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=0; $v=v.1
--> State of array after loop:
array(0) {
}

---( Array with 3 element(s): )---
--> State of referenced array before loop:
array(3) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=0; $v=v.1
     iteration 2:  $k=0; $v=v.2
--> State of array after loop:
array(0) {
}

---( Array with 4 element(s): )---
--> State of referenced array before loop:
array(4) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
  [3]=>
  unicode(3) "v.3"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=0; $v=v.1
     iteration 2:  $k=0; $v=v.2
     iteration 3:  $k=0; $v=v.3
--> State of array after loop:
array(0) {
}



Remove current element of a referenced array
---( Array with 1 element(s): )---
--> State of referenced array before loop:
array(1) {
  [0]=>
  unicode(3) "v.0"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
--> State of array after loop:
array(0) {
}

---( Array with 2 element(s): )---
--> State of referenced array before loop:
array(2) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=1; $v=v.1
--> State of array after loop:
array(0) {
}

---( Array with 3 element(s): )---
--> State of referenced array before loop:
array(3) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=1; $v=v.1
     iteration 2:  $k=2; $v=v.2
--> State of array after loop:
array(0) {
}

---( Array with 4 element(s): )---
--> State of referenced array before loop:
array(4) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
  [3]=>
  unicode(3) "v.3"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=1; $v=v.1
     iteration 2:  $k=2; $v=v.2
     iteration 3:  $k=3; $v=v.3
--> State of array after loop:
array(0) {
}



Adding elements to the end of a referenced array
---( Array with 1 element(s): )---
--> State of referenced array before loop:
array(1) {
  [0]=>
  unicode(3) "v.0"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
--> State of array after loop:
array(2) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(5) "new.0"
}

---( Array with 2 element(s): )---
--> State of referenced array before loop:
array(2) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=1; $v=v.1
     iteration 2:  $k=2; $v=new.0
     iteration 3:  $k=3; $v=new.1
     iteration 4:  $k=4; $v=new.2
     iteration 5:  $k=5; $v=new.3
  ** Stuck in a loop! **
--> State of array after loop:
array(8) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(5) "new.0"
  [3]=>
  unicode(5) "new.1"
  [4]=>
  unicode(5) "new.2"
  [5]=>
  unicode(5) "new.3"
  [6]=>
  unicode(5) "new.4"
  [7]=>
  unicode(5) "new.5"
}

---( Array with 3 element(s): )---
--> State of referenced array before loop:
array(3) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=1; $v=v.1
     iteration 2:  $k=2; $v=v.2
     iteration 3:  $k=3; $v=new.0
     iteration 4:  $k=4; $v=new.1
     iteration 5:  $k=5; $v=new.2
  ** Stuck in a loop! **
--> State of array after loop:
array(9) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
  [3]=>
  unicode(5) "new.0"
  [4]=>
  unicode(5) "new.1"
  [5]=>
  unicode(5) "new.2"
  [6]=>
  unicode(5) "new.3"
  [7]=>
  unicode(5) "new.4"
  [8]=>
  unicode(5) "new.5"
}

---( Array with 4 element(s): )---
--> State of referenced array before loop:
array(4) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
  [3]=>
  unicode(3) "v.3"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=1; $v=v.1
     iteration 2:  $k=2; $v=v.2
     iteration 3:  $k=3; $v=v.3
     iteration 4:  $k=4; $v=new.0
     iteration 5:  $k=5; $v=new.1
  ** Stuck in a loop! **
--> State of array after loop:
array(10) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
  [3]=>
  unicode(3) "v.3"
  [4]=>
  unicode(5) "new.0"
  [5]=>
  unicode(5) "new.1"
  [6]=>
  unicode(5) "new.2"
  [7]=>
  unicode(5) "new.3"
  [8]=>
  unicode(5) "new.4"
  [9]=>
  unicode(5) "new.5"
}



Adding elements to the start of a referenced array
---( Array with 1 element(s): )---
--> State of referenced array before loop:
array(1) {
  [0]=>
  unicode(3) "v.0"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
--> State of array after loop:
array(2) {
  [0]=>
  unicode(5) "new.0"
  [1]=>
  unicode(3) "v.0"
}

---( Array with 2 element(s): )---
--> State of referenced array before loop:
array(2) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=0; $v=new.0
     iteration 2:  $k=0; $v=new.1
     iteration 3:  $k=0; $v=new.2
     iteration 4:  $k=0; $v=new.3
     iteration 5:  $k=0; $v=new.4
  ** Stuck in a loop! **
--> State of array after loop:
array(8) {
  [0]=>
  unicode(5) "new.5"
  [1]=>
  unicode(5) "new.4"
  [2]=>
  unicode(5) "new.3"
  [3]=>
  unicode(5) "new.2"
  [4]=>
  unicode(5) "new.1"
  [5]=>
  unicode(5) "new.0"
  [6]=>
  unicode(3) "v.0"
  [7]=>
  unicode(3) "v.1"
}

---( Array with 3 element(s): )---
--> State of referenced array before loop:
array(3) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=0; $v=new.0
     iteration 2:  $k=0; $v=new.1
     iteration 3:  $k=0; $v=new.2
     iteration 4:  $k=0; $v=new.3
     iteration 5:  $k=0; $v=new.4
  ** Stuck in a loop! **
--> State of array after loop:
array(9) {
  [0]=>
  unicode(5) "new.5"
  [1]=>
  unicode(5) "new.4"
  [2]=>
  unicode(5) "new.3"
  [3]=>
  unicode(5) "new.2"
  [4]=>
  unicode(5) "new.1"
  [5]=>
  unicode(5) "new.0"
  [6]=>
  unicode(3) "v.0"
  [7]=>
  unicode(3) "v.1"
  [8]=>
  unicode(3) "v.2"
}

---( Array with 4 element(s): )---
--> State of referenced array before loop:
array(4) {
  [0]=>
  unicode(3) "v.0"
  [1]=>
  unicode(3) "v.1"
  [2]=>
  unicode(3) "v.2"
  [3]=>
  unicode(3) "v.3"
}
--> Do loop:
     iteration 0:  $k=0; $v=v.0
     iteration 1:  $k=0; $v=new.0
     iteration 2:  $k=0; $v=new.1
     iteration 3:  $k=0; $v=new.2
     iteration 4:  $k=0; $v=new.3
     iteration 5:  $k=0; $v=new.4
  ** Stuck in a loop! **
--> State of array after loop:
array(10) {
  [0]=>
  unicode(5) "new.5"
  [1]=>
  unicode(5) "new.4"
  [2]=>
  unicode(5) "new.3"
  [3]=>
  unicode(5) "new.2"
  [4]=>
  unicode(5) "new.1"
  [5]=>
  unicode(5) "new.0"
  [6]=>
  unicode(3) "v.0"
  [7]=>
  unicode(3) "v.1"
  [8]=>
  unicode(3) "v.2"
  [9]=>
  unicode(3) "v.3"
}