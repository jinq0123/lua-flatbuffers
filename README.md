# lua-flatbuffers
FlatBuffers library for Lua.

Build
------
Need submodule:
```sh
 $ git submodule update --init
```

Use premake5 to generate VS solution and linux Makefile. See premake5.bat.

Vs2015 sln and Makefile are provided.

By default, it expects the Lua library to build under C++.
If you really want to use Lua library compiled under C,
you can define LUAINTF_LINK_LUA_COMPILED_IN_CXX to 0 in build/premake5.lua.
<br>See: https://github.com/SteveKChiu/lua-intf

```
defines { "LUAINTF_LINK_LUA_COMPILED_IN_CXX=0" }
```

Copy lua lib. Copy liblua.a to lua532/lib/.
For Windows, copy lua.lib debug to lua532/lib/Debug,
and lua.lib release to lua532/lib/Release.

Usage Example
--------------
```sh
flatc --binary --schema monster.fbs
```

```lua
local lfb = require("lfb")

-- Load a FlatBuffers schema file.
assert(lfb.load_bfbs_file("monster.bfbs"))

local monster = {
  pos = {
    x = 1,
    y = 2,
    z = 3,
  },
  hp = 300,
  name = "Orc",
}

-- Build a buffer.
local buf = assert(lfb.encode("Monster", monster))

-- Decode a flatbuffer string back to a Lua table.
local monster2 = assert(lfb.decode("Monster", buf))
```

Test
--------
<pre>
E:\Git\Lua\lua-flatbuffers_jinq0123\test>lua53pp.exe test.lua
All test passed.

E:\Git\Lua\lua-flatbuffers_jinq0123\test>
</pre>

<pre>
E:\Git\Lua\lua-flatbuffers_jinq0123\test>lua53pp.exe
Lua 5.3.2  Copyright (C) 1994-2015 Lua.org, PUC-Rio
> package.cpath = package.cpath .. ";../bin/Debug/?.dll"
> lfb = require("lfb")
> lfb.load_bfbs_file("../third_party/flatbuffers/tests/monster_test.bfbs")
true
> buf = assert(lfb.encode("Monster", {name="N", hp=1234}))
> t = assert(lfb.decode("Monster", buf))
> inspect = require("inspect")
> inspect(t)
{
  color = 8,
  hp = 1234,
  mana = 150,
  name = "N",
  test_type = 0,
  testbool = false,
  testf = 3.1415901184082,
  testf2 = 3.0,
  testf3 = 0.0,
  testhashs32_fnv1 = 0,
  testhashs32_fnv1a = 0,
  testhashs64_fnv1 = 0,
  testhashs64_fnv1a = 0,
  testhashu32_fnv1 = 0,
  testhashu32_fnv1a = 0,
  testhashu64_fnv1 = 0,
  testhashu64_fnv1a = 0
}
>
</pre>

Type Convertion
--------------------
Number will be converted to string if necessary. 
```lua
	buf = assert(lfb.encode("Monster", {name=123}))
	t = assert(lfb.decode("Monster", buf))
	assert("123" == t.name)
```

Integer will be converted from one type to another.
  Such as from int 256 to byte 0:
```lua
	buf = assert(lfb.encode("Test", {a=1, b=256}))  -- Test.b is byte
	t = assert(lfb.decode("Test", buf))
	assert(1 == t.a and 0 == t.b)
```

String can convert to integer or float:
```lua
	buf = assert(lfb.encode("Test", {a=1, b="25"}))
	t = assert(lfb.decode("Test", buf))
	assert(1 == t.a and 25 == t.b)
	buf = assert(lfb.encode("Monster", {name="", testf="1.2"}))
	t = assert(lfb.decode("Monster", buf))
	assert(math.type(t.testf) == "float")
	assert(tostring(t.testf) == "1.2000000476837")
```

Can not convert float to integer.
```lua
	buf, err = lfb.encode("Test", {a=1.2, b=2})
	assert(err == "can not convert field Test.a(1.2) to integer")
```

Enum is integer, but input string enum will be converted to integer.
```lua
	local name = "TestSimpleTableWithEnum"
	buf = assert(lfb.encode(name, {color = 123}))
	t = assert(lfb.decode(name, buf))
	assert(123 == t.color)

	buf = assert(lfb.encode(name, {color = "Blue"}))
	t = assert(lfb.decode(name, buf))
	assert(8 == t.color)
```

Array only read from index 1 to len, ignoring others.
```lua
	buf = assert(lfb.encode("Monster", {name="", inventory={
		1,2, [-1]=-1, [100]=100, x=101}}))
	t = assert(lfb.decode("Monster", buf))
	assert(2 == #t.inventory)
	assert(nil == t.inventory[-1])
	assert(nil == t.inventory[100])
```

Todo
------
* Support namespace.
  Reflection schema does not support namespaces #3899 ( https://github.com/google/flatbuffers/issues/3899 )
* Sort key.
* Verify key order.
* Add file_identifier if root_type.  
* Load fbs file directly.

DavidFeng/lua-flatbuffers
-------------------------
https://github.com/DavidFeng/lua-flatbuffers

Another FlatBuffers library for lua 5.3,
which implemented the reading of FlatBuffers
but writing is to be implemented.
