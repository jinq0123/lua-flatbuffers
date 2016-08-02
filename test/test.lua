package.cpath = package.cpath .. ";../bin/Debug/?.dll"

lfb = require("lfb")
inspect = require("inspect")

assert(lfb.load_bfbs_file("../third_party/flatbuffers/tests/monster_test.bfbs"))

function test_required()
	buf = assert(lfb.encode("Monster", {}))
	t, err = lfb.decode("Monster", buf)
	assert(err == "illegal required field Monster.name")
end  -- test_required()

function test_too_short()
	TO_SHORT = "buffer is too short"
	t, err = lfb.decode("Monster", "")
	assert(err == TO_SHORT)
	t, err = lfb.decode("Monster", "123")
	assert(err == TO_SHORT)
	assert(not lfb.decode("Monster", "1234"))
	assert(not lfb.decode("Monster", "1234"))
end

function test_all()
	test_required()
	test_too_short()
end  -- test_all()

test_all()
