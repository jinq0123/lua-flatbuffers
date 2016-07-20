package.cpath = package.cpath .. ";../bin/Debug/?.dll"

fb = require("flatbuffers")
assert(fb.load_bfbs_file("../third_party/flatbuffers/tests/monster_test.bfbs"))

buf = assert(fb.encode("Monster", {}))
