--TEST--
SOAP XML Schema 15: simpleType/union (inline type)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
include "test_schema.inc";
$schema = <<<EOF
	<simpleType name="testType">
		<union>
			<simpleType>
				<restriction base="string"/>
			</simpleType>
			<simpleType>
				<restriction base="int"/>
			</simpleType>
			<simpleType>
				<restriction base="float"/>
			</simpleType>
		</union>
	</simpleType>
EOF;
test_schema($schema,'type="tns:testType"',"str");
echo "ok";
?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://test-uri/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:test><testParam xsi:type="ns1:testType">str</testParam></ns1:test></SOAP-ENV:Body></SOAP-ENV:Envelope>
unicode(3) "str"
ok
