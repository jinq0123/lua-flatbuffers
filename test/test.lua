package.cpath = package.cpath .. ";../bin/Debug/?.dll"

fb = require("flatbuffers")
assert(fb.load_bfbs_file("../third_party/flatbuffers/tests/monster_test.bfbs"))

buf = assert(fb.encode("Monster", {}))
t = assert(fb.decode("Monster", buf))

TO_SHORT = "buffer is too short"
t, err = fb.decode("Monster", "")
assert(err == TO_SHORT)
t, err = fb.decode("Monster", "123")
assert(err == TO_SHORT)

