#pragma once
enum class MAGFilter : int
{
	NEAREST , LINEAR
};
enum class MINFilter : int
{
	NONE , MIPMAP_LINEAR , MIPMAP_CLOSEST
};
enum class WrapRegime
{
	CLAMP , REPEAT , MIRROR
};
enum class PixelType : int
{
	FLOAT , BYTE , INT , FIVE
};
enum class PixelMapping : int
{
	RGB , RGBA , BGRA , BGR , R
};
struct BitMap2D
{
	void *data;
	int width;
	int height;
	PixelMapping pixel_mapping;
	PixelType pixel_type;
	uint getBpp() const
	{
		uint component_size;
		switch( pixel_type )
		{
		case PixelType::BYTE:
			component_size = 1;
			break;
		case PixelType::FLOAT:
		case PixelType::INT:
			component_size = 4;
			break;
		case PixelType::FIVE:
			return 2;
			break;
		}
		switch( pixel_mapping )
		{
		case PixelMapping::BGR:
		case PixelMapping::RGB:
			return 3 * component_size;
		case PixelMapping::R:
			return component_size;
		case PixelMapping::BGRA:
		case PixelMapping::RGBA:
			return 4 * component_size;
		}
	}
};
struct TextureDesc
{
	BitMap2D bitmap;
	MAGFilter mag_filter;
	MINFilter min_filter;
	WrapRegime x_regime;
	WrapRegime y_regime;
};