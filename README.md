lua-struct
==========

A Lua clone of the python.struct module.

Summary
-------

lua-struct is a clone of the python.struct module.
This package enables Lua to read/write binary data in a comfortable way.

Example
-------
```lua
local bindata = struct.pack('s4hf', 'PACK', 1024, math.pi)
```
bindata will contain a 4 byte "string" with PACK, a singed short with 1024
and a 8 byte float with PI. Now we're going to unpack it...
```lua
print(struct.unpack('s4hf', bindata))
```

Installation
------------

Need lua 5.2, then just "make" it.


Bugs
----
 * no bugs I'm aware of...please find some ;)

Todo
----
 * add more datatypes (e.g. long, long long)

If you have some particular request, just send me an email, and I will see for it!

