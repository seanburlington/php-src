--TEST--
Bug #38465 (ReflectionParameter fails on access to self::)
--FILE--
<?php
class Baz {
    const B = 3;
}

class Foo {
    const X = 1;
    private $propA = self::X;
    private $propB = Baz::B;
    private $propC = 99;
    public function x($a = self::X, $b = Baz::B, $c = 99) {}
}

class Bar extends Foo {
    const Y = 2;
    private $propA = self::X;
    private $propB = Baz::B;
    private $propC = 99;
    public function y($a = self::Y, $b = Baz::B, $c = 99) {}
}


echo "From global scope:\n";

$clazz = new ReflectionClass('Bar');
foreach ($clazz->getProperties() as $property) {
    echo $property->getDeclaringClass()->getName(), '::$', $property->getName(), ' = ', $property->getDefaultValue(), "\n";
}

foreach ($clazz->getMethods() as $method) {
    foreach ($method->getParameters() as $param) {
        if ($param->isDefaultValueAvailable()) {
            echo $method->getDeclaringClass()->getName(), '::', $method->getName(), '($', $param->getName(), ' = ', $param->getDefaultValue(), ")\n";
        }
    }
}

echo "\nFrom class context:\n";

class Test {
    function __construct() {
        $clazz = new ReflectionClass('Bar');
        foreach ($clazz->getProperties() as $property) {
            echo $property->getDeclaringClass()->getName(), '::$', $property->getName(), ' = ', $property->getDefaultValue(), "\n";
        }

        foreach ($clazz->getMethods() as $method) {
            foreach ($method->getParameters() as $param) {
                if ($param->isDefaultValueAvailable()) {
                    echo $method->getDeclaringClass()->getName(), '::', $method->getName(), '($', $param->getName(), ' = ', $param->getDefaultValue(), ")\n";
                }
            }
        }
    }
}

new Test();

?>
--EXPECT--
From global scope:
Bar::$propA = 1
Bar::$propB = 3
Bar::$propC = 99
Bar::y($a = 2)
Bar::y($b = 3)
Bar::y($c = 99)
Foo::x($a = 1)
Foo::x($b = 3)
Foo::x($c = 99)

From class context:
Bar::$propA = 1
Bar::$propB = 3
Bar::$propC = 99
Bar::y($a = 2)
Bar::y($b = 3)
Bar::y($c = 99)
Foo::x($a = 1)
Foo::x($b = 3)
Foo::x($c = 99)
