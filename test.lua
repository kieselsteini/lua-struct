local struct = require('struct')    -- get module
-- some info
print('supported format chars: ' .. struct.formatchars)
print('system byteorder:       ' .. struct.byteorder)
-- tests
assert(struct.unpack('b', struct.pack('b', -64)) == -64, 'b')
assert(struct.unpack('B', struct.pack('B', 255)) == 255, 'B')
assert(struct.unpack('@h', struct.pack('@h', -1024)) == -1024, '@h')
assert(struct.unpack('<h', struct.pack('<h', -1024)) == -1024, '<h')
assert(struct.unpack('>h', struct.pack('>h', -1024)) == -1024, '>h')
assert(struct.unpack('@H', struct.pack('@H', 50000)) ==  50000, '@H')
assert(struct.unpack('<H', struct.pack('<H', 50000)) ==  50000, '<H')
assert(struct.unpack('>H', struct.pack('>H', 50000)) ==  50000, '>H')
assert(struct.unpack('@i', struct.pack('@i', -1024)) == -1024, '@i')
assert(struct.unpack('<i', struct.pack('<i', -1024)) == -1024, '<i')
assert(struct.unpack('>i', struct.pack('>i', -1024)) == -1024, '>i')
assert(struct.unpack('@I', struct.pack('@I', 50000)) ==  50000, '@I')
assert(struct.unpack('<I', struct.pack('<I', 50000)) ==  50000, '<I')
assert(struct.unpack('>I', struct.pack('>I', 50000)) ==  50000, '>I')
-- floats can't be tested that way :/
assert(math.abs(math.pi - struct.unpack('f', struct.pack('f', math.pi))) < 0.0000001, 'f')
assert(struct.unpack('d', struct.pack('d', math.pi)) == math.pi, 'd')
-- paddings
assert(struct.unpack('xxi', struct.pack('xxi', 32)) == 32, 'padding')
-- strings
assert(struct.unpack('s5', struct.pack('s5', 'hello')) == 'hello', 's')
-- size
assert(struct.size('ihH') == 8, 'size(ihH)')
-- complex example
print(struct.unpack('<s4Ixh', struct.pack('<s4Ixh', 'WAV', 1024, 512)))

