# lua-flatbuffers
FlatBuffers library for Lua.

(Not ready.)

Build
------
Need submodule:
```sh
 $ git submodule update --init
```

Use premake5 to generate VS solution and linux Makefile. See premake5.bat.

By default, it expect the Lua library to build under C++.
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
local buf = assert(lfb:encode("Monster", monster))

-- Decode a flatbuffer string back to a Lua table.
local monster2 = assert(lfbs:decode("Monster", buf))
```

Test
--------
<pre>
E:\Git\Lua\lua-flatbuffers_jinq0123\test>lua53pp.exe
Lua 5.3.2  Copyright (C) 1994-2015 Lua.org, PUC-Rio
> package.cpath = package.cpath .. ";../bin/Debug/?.dll"
> lfb = require("lfb")
> lfb.load_bfbs_file("../third_party/flatbuffers/tests/monster_test.bfbs")
true
> buf = lfb.encode("Monster", {hp = 1234})
> t = lfb.decode("Monster", buf)
> inspect = require("inspect")
> inspect(t)
{
  color = 8,
  hp = 1234,
  mana = 150,
  test_type = 0,
  testbool = 0,
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

* Number will be converted to string if necessary. 
* Number will be converted from one type to another, such as from float to byte.

Enum is integer, and converts string enum to integer automatically.
```
	local name = "TestSimpleTableWithEnum"
	buf = assert(lfb.encode(name, {color = 123}))
	t = assert(lfb.decode(name, buf))
	assert(123 == t.color)

	buf = assert(lfb.encode(name, {color = "Blue"}))
	t = assert(lfb.decode(name, buf))
	assert(8 == t.color)
```

Todo
------
* Support namespace.
  Reflection schema does not support namespaces #3899 ( https://github.com/google/flatbuffers/issues/3899 )

DavidFeng/lua-flatbuffers
-------------------------
https://github.com/DavidFeng/lua-flatbuffers

Another FlatBuffers library for lua 5.3,
which implemented the reading of FlatBuffers
but writing is to be implemented.
