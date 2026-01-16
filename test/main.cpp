#include <pngimg.hpp>
// #include <iostream>

static void	fillImage(png::image& img, int maxX);
static void	fillImage(png::image& img, int maxX, int maxY);
static void	printColorSet(png::image& img, png::ColorType col, png::BitDepth bd);

int main(void)
{
	png::image	img;

	fillImage(img, 255);

	// Print per Type
	printColorSet(img, png::ColorType::GRAYSCALE, png::BitDepth::BIT_1);
	printColorSet(img, png::ColorType::GRAYSCALE, png::BitDepth::BIT_2);
	printColorSet(img, png::ColorType::GRAYSCALE, png::BitDepth::BIT_4);
	printColorSet(img, png::ColorType::GRAYSCALE, png::BitDepth::BIT_8);
	printColorSet(img, png::ColorType::GRAYSCALE, png::BitDepth::BIT_16);
	printColorSet(img, png::ColorType::TRUECOLOR, png::BitDepth::BIT_8);
	printColorSet(img, png::ColorType::TRUECOLOR, png::BitDepth::BIT_16);
	// printColorSet(img, png::ColorType::PALETTE, png::BitDepth::BIT_1);
	// printColorSet(img, png::ColorType::PALETTE, png::BitDepth::BIT_2);
	// printColorSet(img, png::ColorType::PALETTE, png::BitDepth::BIT_4);
	// printColorSet(img, png::ColorType::PALETTE, png::BitDepth::BIT_8);
	printColorSet(img, png::ColorType::GRAYSCALE_ALPHA, png::BitDepth::BIT_8);
	printColorSet(img, png::ColorType::GRAYSCALE_ALPHA, png::BitDepth::BIT_16);
	printColorSet(img, png::ColorType::TRUECOLOR_ALPHA, png::BitDepth::BIT_8);
	printColorSet(img, png::ColorType::TRUECOLOR_ALPHA, png::BitDepth::BIT_16);
}

static void	fillImage(png::image& img, int maxX)
{
	fillImage(img, maxX, maxX * 3 / 4);
}

static void	fillImage(png::image& img, int maxX, int maxY)
{
	img.unsetMap();
	for (int x = 0; x < maxX; ++x)
	{
		for (int y = 0; y < maxY; ++y)
		{
			// std::cout	<< x << '/' << y << ' ';
			img[x][y] = png::rgba(255, x * 255 / maxX, y * 255 / maxY, 127);
		}
		// std::cout	<< '\n';
	}
	// std::cout	<< std::flush;
}

static void	printColorSet(png::image& img, png::ColorType col, png::BitDepth bd)
{
	std::string	name = "test/test-" + std::to_string(static_cast<int>(col)) + "-" + std::to_string(static_cast<int>(bd)) + ".png";
	img.setColorType(col);
	img.setBitDepth(bd);
	img.save(name.c_str());
}
