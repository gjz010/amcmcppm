#pragma once
#include "defs.h"
#include "vector"
class ImageWriter
{
public:
	ImageWriter();
	static bool write(const std::string& path,std::vector<vec3> image, int width, int height);
	static bool read(const wchar_t* path, std::vector<vec3>& image, int& width, int& height);
	~ImageWriter();
};

