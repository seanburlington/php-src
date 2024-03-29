--TEST--
Interface of the class mysqli_result - Reflection
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipifemb.inc');
require_once('skipifconnectfailure.inc');
require_once('connect.inc');

if (($tmp = substr(PHP_VERSION, 0, strpos(PHP_VERSION, '.'))) && ($tmp < 5))
	die("skip Reflection not available before PHP 5 (found PHP $tmp)");

/*
Let's not deal with cross-version issues in the EXPECTF/UEXPECTF.
Most of the things which we test are covered by mysqli_class_*_interface.phpt.
Those tests go into the details and are aimed to be a development tool, no more.
*/
if (!$IS_MYSQLND)
	die("skip Test has been written for the latest version of mysqlnd only");
if ($MYSQLND_VERSION < 576)
	die("skip Test requires mysqlnd Revision 576 or newer");
?>
--FILE--
<?php
	require_once('reflection_tools.inc');
	$class = new ReflectionClass('mysqli_result');
	inspectClass($class);
	print "done!";
?>
--EXPECTF--
Inspecting class 'mysqli_result'
isInternal: yes
isUserDefined: no
isInstantiable: yes
isInterface: no
isAbstract: no
isFinal: no
isIteratable: no
Modifiers: '0'
Parent Class: ''
Extension: 'mysqli'

Inspecting method '__construct'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: yes
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method '__construct'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: yes
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'close'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'data_seek'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'fetch_all'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'fetch_array'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'fetch_assoc'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'fetch_field'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'fetch_field_direct'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'fetch_fields'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'fetch_object'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'fetch_row'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'field_seek'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'free'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting method 'free_result'
isFinal: no
isAbstract: no
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isConstructor: no
isDestructor: no
isInternal: yes
isUserDefined: no
returnsReference: no
Modifiers: %d
Number of Parameters: 0
Number of Required Parameters: 0

Inspecting property 'current_field'
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isDefault: yes
Modifiers: 256

Inspecting property 'field_count'
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isDefault: yes
Modifiers: 256

Inspecting property 'lengths'
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isDefault: yes
Modifiers: 256

Inspecting property 'num_rows'
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isDefault: yes
Modifiers: 256

Inspecting property 'type'
isPublic: yes
isPrivate: no
isProtected: no
isStatic: no
isDefault: yes
Modifiers: 256
Default property 'current_field'
Default property 'field_count'
Default property 'lengths'
Default property 'num_rows'
Default property 'type'
done!
