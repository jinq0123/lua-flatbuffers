# lua-flatbuffers
FlatBuffers library for Lua.

(Not ready.)

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
