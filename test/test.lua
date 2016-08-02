package.cpath = package.cpath .. ";../bin/Debug/?.dll"

lfb = require("lfb")
inspect = require("inspect")

assert(lfb.load_bfbs_file("../third_party/flatbuffers/tests/monster_test.bfbs"))

function test_no_type()
	buf, err = lfb.encode("Abcd", {})
	assert(err == "no type Abcd")
end

function test_required()
	buf = assert(lfb.encode("Monster", {}))
	t, err = lfb.decode("Monster", buf)
	assert(err == "illegal required field Monster.name")

	buf = assert(lfb.encode("Monster", {name="abc"}))
	t = assert(lfb.decode("Monster", buf))
	assert(t.name == "abc")
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

function test_not_table()
	buf, err = lfb.encode("Monster", nil)
	assert(nil == buf)
	assert(err == "lua data is not table but nil")
	buf, err = lfb.encode("Monster", 1234)
	assert(err == "lua data is not table but number")
	buf, err = lfb.encode("Monster", print)
	assert(err == "lua data is not table but function")
end  -- test_not_table()

function test_string_field()
	buf, err = lfb.encode("Monster", {name=123})
	assert(err == "string field Monster.name is number")
	buf, err = lfb.encode("Monster", {name=print})
	assert(err == "string field Monster.name is function")
	assert(lfb.encode("Monster", {name=""}))
end  -- test_string_field()

function test_encode_struct()
	buf, err = lfb.encode("Test", {})
	assert(err == "missing struct field Test.a")
	buf, err = lfb.encode("Test", {a=1})
	assert(err == "missing struct field Test.b")
	buf, err = lfb.encode("Test", {a=1, b=2, c=3})
	assert(err == "illegal field Test.c")
	buf = assert(lfb.encode("Test", {a=1, b=2}))
	t = assert(lfb.decode("Test", buf))
	assert(t.a == 1 and t.b == 2)
	buf, err = lfb.encode("Test", {a=1, b={}})
	assert(err == "Byte struct field Test.b is table")
end  -- test_encode_struct()

function test_all()
	test_no_type()
	test_required()
	test_too_short()
	test_not_table()
	test_string_field()
	test_encode_struct()
end  -- test_all()

test_all()
