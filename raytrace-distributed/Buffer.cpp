#include <vector>
#include "Buffer.h"


Buffer::Buffer()
{

}


Buffer::~Buffer()
{
}

void Buffer::append(const Buffer & buf)
{
	buffer.insert(buffer.end(), buf.buffer.begin(), buf.buffer.end());
}

void Buffer::append(byte b)
{
	buffer.push_back(b);
}

byte & Buffer::operator[](int index)
{
	//return *(new byte);
	return buffer[index];
}

int Buffer::size()
{
	//return 0;
	return buffer.size();
}

Buffer::Reader Buffer::reader()
{
	Reader r;
	
	r.iter = buffer.begin();
	r.iend = buffer.end();
	
	return r;
}


bool Buffer::Reader::hasNext()
{
	//return true;
	return iter == iend;
}

byte Buffer::Reader::next()
{
	if(hasNext())
		return *(iter++);
	return 0;
}
