#pragma once

enum ByteType {
	BYTE,
	WILDCARD
};

struct PatternByte {
	unsigned char byte;
	ByteType byteType;
};

