#include <pngimg.hpp>
// #include <iostream>

static void	printColorField();
static void	fillImage(png::image& img, int maxX);
static void	fillImage(png::image& img, int maxX, int maxY);
static void	printColorSet(png::image& img, png::ColorType col, png::BitDepth bd);

static void	printLines();

static void	printShapes();

int main(void)
{
	// printColorField();
	printLines();
	printShapes();
}

static void	printColorField()
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
#include <cmath>
#include <iostream>
static void	printLines()
{
	png::image							img;
	std::vector<std::pair<int, int>>	line;

	int 	polygon = 16;
	double	radius = 100;
	double	angleStep = 2 * M_PI / polygon;
	for (int i = 0; i < polygon + 1; ++i)
	{
		line.push_back({
			radius + radius * std::cos(angleStep * (i + 0.5)),
			radius + radius * std::sin(angleStep * (i + 0.5))
		});
	}
	img.drawLine(line, {255, 127, 0});
	img.save("test/linePolygon.png");
}

static void	printShapes()
{
	png::image	img;

	int		centerX = 430;
	int		centerY = 260;
	double	ratio = double(centerX) / centerY;
	for (int size = 0; size <= centerY; size += 2)
	{
		png::rgba	color(0, 0, 0);
		switch (size % 3)
		{
			case 0:	color.r = 255;	break;
			case 1:	color.g = 255;	break;
			default:	color.b = 255;	break;
		}
		img.drawEllipse(centerX - int(size * ratio), centerY - size, centerX + int(size * ratio), centerY + size, color);
	}
	img.save("test/testEllipse.png");
}