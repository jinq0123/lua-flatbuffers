package.cpath = package.cpath .. ";../bin/Debug/?.dll"

lfb = require("lfb")
assert(lfb.load_bfbs_file("../third_party/flatbuffers/tests/monster_test.bfbs"))

buf = assert(lfb.encode("Monster", {}))
t = assert(lfb.decode("Monster", buf))

TO_SHORT = "buffer is too short"
t, err = lfb.decode("Monster", "")
assert(err == TO_SHORT)
t, err = lfb.decode("Monster", "123")
assert(err == TO_SHORT)
assert(not lfb.decode("Monster", "1234"))
