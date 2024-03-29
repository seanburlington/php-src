--TEST--
SOAP XML Schema 19: union with list
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
include "test_schema.inc";
$schema = <<<EOF
	<simpleType name="testType">
		<union>
			<simpleType>
				<restriction base="float"/>
			</simpleType>
			<simpleType>
				<list itemType="int"/>
			</simpleType>
		</union>
	</simpleType>
EOF;
test_schema($schema,'type="tns:testType"',"123.5 456.7");
echo "ok";
?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://test-uri/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:test><testParam xsi:type="ns1:testType">123.5 456.7</testParam></ns1:test></SOAP-ENV:Body></SOAP-ENV:Envelope>
unicode(11) "123.5 456.7"
ok
