#include "ImageWriter.h"
#include <png.h>


ImageWriter::ImageWriter()
{
}
unsigned char real2byte(real p) {
	if (p < 0) p = 0;
	else if (p > 1.0) p = 1.0;
	return round(p * 255);
}

bool ImageWriter::write(const std::string & path, std::vector<vec3> image, int width, int height)
{
	FILE* fp = fopen(path.c_str(), "wb");
	if (!fp) return false;
	png_structp png_ptr = nullptr;
	png_infop info_ptr = nullptr;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) return false;
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, nullptr);
	}
	int err_jmp = setjmp(png_jmpbuf(png_ptr));
	if (err_jmp) return false;
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr,
		width, height,
		8,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE);
	png_set_packing(png_ptr);
	png_write_info(png_ptr, info_ptr);
	unsigned char** buffer = new unsigned char*[sizeof(unsigned char*)*height];
	int len = width * 3;
	int index = 0;
	for (int i = 0; i < height; i++) {
		buffer[i] = new unsigned char[sizeof(unsigned char)*len];
		for (int j = 0; j < len; j += 3,index++) {
			buffer[i][j] = real2byte(image[index][0]);
			buffer[i][j+1] = real2byte(image[index][1]);
			buffer[i][j+2] = real2byte(image[index][2]);
		}
	}
	png_write_image(png_ptr, buffer);
	for (int i = 0; i < height; i++) delete[] buffer[i];
	delete[] buffer;
	fclose(fp);
	return 0;
}

bool ImageWriter::read(const wchar_t * path, std::vector<vec3>& image, int & width, int & height)
{
	FILE* fp = _wfopen(path, L"rb");
	if (!fp) return false;
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) return false;

	png_infop info = png_create_info_struct(png);
	if (!info) return false;

	if (setjmp(png_jmpbuf(png))) return false;

	png_init_io(png, fp);

	png_read_info(png, info);

	width = png_get_image_width(png, info);
	height = png_get_image_height(png, info);
	char color_type = png_get_color_type(png, info);
	char bit_depth = png_get_bit_depth(png, info);

	// Read any color_type into 8bit depth, RGBA format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	if (bit_depth == 16)
		png_set_strip_16(png);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	if (png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

	// These color_type don't have an alpha channel then fill it with 0xff.
	if (color_type == PNG_COLOR_TYPE_RGB ||
		color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);

	png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for (int y = 0; y < height; y++) {
		row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
	}

	png_read_image(png, row_pointers);

	for (int y = height-1; y >=0; y--) {
		for (int x = 0; x < width; x++) {
			vec3 c;
			c[0] = row_pointers[y][x * 4];
			c[1] = row_pointers[y][x * 4 + 1];
			c[2] = row_pointers[y][x * 4 + 2];
			image.push_back(c/255);

		}
		delete[] row_pointers[y];
	}
	delete[] row_pointers;
	fclose(fp);
}




ImageWriter::~ImageWriter()
{
}
