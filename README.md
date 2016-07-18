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
you can define LUAINTF_LINK_LUA_COMPILED_IN_CXX to 0.
<br>See: https://github.com/SteveKChiu/lua-intf

```C++
// #define LUAINTF_LINK_LUA_COMPILED_IN_CXX 0
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
local fb = require("flatbuffers")

-- Create a FlatBuffers schema object.
local fbs = fb.load_bfbs("monster.bfbs")

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
local buf = fbs:encode(monster)

-- Decode a flatbuffer string back to a Lua table.
local monster2 = fbs:decode(buf)
```

DavidFeng/lua-flatbuffers
-------------------------
https://github.com/DavidFeng/lua-flatbuffers

Another FlatBuffers library for lua 5.3,
which implemented the reading of FlatBuffers.
Writing is to be implemented.
