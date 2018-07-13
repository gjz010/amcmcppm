#pragma once
#include "defs.h"

#include <vector>
class Buffer
{
public:
	Buffer();
	~Buffer();
	void append(const Buffer& buf);
	void append(byte b);
	byte& operator[](int index);
	int size();

private:
	std::vector<byte> buffer;

	struct Reader {
	private:
		std::vector<byte>::iterator iter;
		std::vector<byte>::iterator iend;
	public:
		friend class Buffer;
		bool hasNext();
		byte next();
	};
public:
	Reader reader();
};

