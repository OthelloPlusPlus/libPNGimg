#include "../include/pngimg.hpp"

#include <iostream>	// std::cerr
#include <cstring>	// std::memcmp, std::memset
#include <zlib.h>	// compression
#include <ctime>	// std::time
#include <sys/utsname.h>	// utsname

#ifndef SOFTWARENAME
# define SOFTWARENAME	"libPNG"
#endif

#ifndef WEBSITE
# define WEBSITEURL	"https://github.com/OthelloPlusPlus"
#endif

#pragma region "Helper functions"

namespace
{

tm	getUTC()
{
	time_t	t = std::time(nullptr);
	std::tm	utc;

#if defined(_WIN32) || defined(_WIN64)
	gmtime_s(&utc, &t);
#elif defined(_POSIX_VERSION)
	gmtime_r(&t, &utc);
#else
	std::tm*	temp = std::gmtime(&t);
	if (temp)
		utc = *temp;
	else
		std::memset(&utc, 0, sizeof(utc));
#endif
	return (utc);
}


}

#pragma endregion "Helper functions"


namespace png
{
	namespace MetaData
	{
		const char*	Title			= "Title";
		const char*	Author			= "Author";
		const char*	Description		= "Description";
		const char*	Copyright		= "Copyright";
		const char*	CreationTime	= "Creation Time";
		const char*	Software		= "Software";
		const char*	Disclaimer		= "Disclaimer";
		const char*	Warning			= "Warning";
		const char*	Source			= "Source";
		const char*	Comment			= "Comment";
	}


image::image()
{
	this->colorType = ColorType::TRUECOLOR_ALPHA;
	this->setBitDepth(BitDepth::BIT_8);
}

image::image(const char* fileToLoad)
{
	this->load(fileToLoad);
}

image::~image()
{
	this->unsetMap();
	this->unsetBackgroundColor();
	this->clearMetaData();
}

////////////////////////////////////////////////////////////////////////////////

void	image::setBackgroundColor(rgba color)
{
	if (this->backgroundColor != nullptr)
		this->unsetBackgroundColor();
	this->backgroundColor = new rgba(color);
}

void	image::setBackgroundColor(short r, short g, short b)
{
	this->setBackgroundColor(rgba(r, g, b, 255));
}
void	image::setBackgroundColor(short r, short g, short b, short a)
{
	this->setBackgroundColor(rgba(r, g, b, a));
}

void	image::unsetBackgroundColor()
{
	if (!this->backgroundColor)
		return ;
	delete this->backgroundColor;
	this->backgroundColor = nullptr;
}

void	image::setColorType(ColorType	type)
{
	this->colorType = type;
	this->calculateChannels();
	this->setBitDepth(this->bitDepth);
}

void	image::calculateChannels()
{
	this->channels = 1;

	uint8_t	ct = (uint8_t)this->colorType;
	if ((ct & 0b1) == 0)
		this->channels += (ct & 0b10) + (ct >> 2);
}

void	image::setBitDepth(BitDepth depth)
{
	switch (this->colorType)
	{
		case ColorType::GRAYSCALE:
			break;
		case ColorType::TRUECOLOR:
			if (depth < BitDepth::BIT_8)
				depth = BitDepth::BIT_8;
			break;
		case ColorType::PALETTE:
			if (depth > BitDepth::BIT_8)
				depth = BitDepth::BIT_8;
			break;
		case ColorType::GRAYSCALE_ALPHA:
			if (depth < BitDepth::BIT_8)
				depth = BitDepth::BIT_8;
			break;
		case ColorType::TRUECOLOR_ALPHA:
			if (depth < BitDepth::BIT_8)
				depth = BitDepth::BIT_8;
			break;
		default:
			break;
	}
	this->bitDepth = depth;
}

void	image::unsetMap()
{
	for (auto i = this->pixels.begin(); i != this->pixels.end(); ++i)
		i->second.clear();
	this->pixels.clear();
	for(auto i = this->printMap.begin(); i != this->printMap.end(); ++i)
		i->second.clear();
	this->printMap.clear();
}

void	image::addMetaData(MetaKey keyword, const char* text)
{
	this->addMetaData(keyword.key, text);
}

void	image::addMetaData(const char* keyword, const char* text)
{
	// Validate text
	if (!text || text[0] == '\0')
		return ;

	// Validate keyword
	std::string	key(keyword);

	if (key.find('\0') != key.npos)
		return ;
	key.erase(0, key.find_first_not_of(" \t\n\r"));
	key.erase(key.find_last_not_of(" \t\n\r") + 1);
	if (key.length() < 1 || key.length() > 79)
		return ;

	// Add to metaData
	this->metaData[key] = text;
}

void	image::rmMetaData(MetaKey keyword)
{
	this->rmMetaData(keyword.key);
}

void	image::rmMetaData(const char* keyword)
{
	if (keyword)
		this->metaData.erase(keyword);
}

void	image::clearMetaData()
{
	this->metaData.clear();
}

// #ifdef DEBUG
void	image::printMetaData()
{
	for (auto& mapped : this->metaData)
		std::cout	<< mapped.first	<< ":\t"	<< mapped.second	<< '\n';
	std::cout	<< std::flush;
}

// #endif

////////////////////////////////////////////////////////////////////////////////

#pragma region	"Load functions"

bool	image::load(const char* name)
{
	try
	{
		image::File		fileClass(name);
		std::fstream&	fileStream = fileClass.open(std::fstream::in | std::fstream::binary);

		if (!image::Signature_Confirm(fileStream))
			throw std::runtime_error("File doesn't start with PNG signature");
		this->clearMetaData();
		while (this->readChunk(fileStream)) {}
	}
	catch(const std::exception& e)
	{
		std::cerr	<< "PNG Error";
		if (errno)
			std::cerr	<< '['	<< errno	<< strerror(errno)	<< ']';
		std::cerr	<< ": "	<< e.what()	<< std::endl;;
		return (false);
	}
	return (true);
}

bool	image::Signature_Confirm(std::fstream& file)
{
	const unsigned char	signature[] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
	char				buffer[8];

	file.read(buffer, 8);
	if (std::memcmp(signature, buffer, 8))
		return (false);
	return (true);
}

#define TWOBYTE_BIGENDIAN(s)	(uint32_t((s)[0])<<24 | \
								uint32_t((s)[1])<<16 | \
								uint32_t((s)[2])<<8 | \
								uint32_t((s)[3]))
bool	image::readChunk(std::fstream& file)
{
	image::Chunk	chunk;

	chunk.readFrom(file);
	std::vector<uint8_t>	data = chunk.getData();
	switch (TWOBYTE_BIGENDIAN(chunk.getType()))
	{
		case TWOBYTE_BIGENDIAN("IHDR"):
			this->IHDR_Load(data.data());
			break;
		case TWOBYTE_BIGENDIAN("IDAT"):
			{
				unsigned long size = data.size();
				uint8_t*	uData = this->IDAT_UncompressData(data.data(), size);
				this->IDAT_Load(uData, size);
				delete[] uData;
			}
			break;
		case TWOBYTE_BIGENDIAN("tEXt"):
			this->tEXt_Load(data.data(), data.size());
			break;
		case TWOBYTE_BIGENDIAN("tIME"):
			// Skipped, because irrelevant.
			break;
		case TWOBYTE_BIGENDIAN("IEND"):
			return (false);
			break;
		default:
			std::cerr	<< "Unknown Chunk type: "	<< std::string(chunk.getType(), 4)	<< std::endl;
			break;
	}
	return (true);
}

uint32_t	image::readChunk4Bytes(std::fstream& file)
{
	uint8_t	buffer[4];

	if (!file.read(reinterpret_cast<char*>(buffer), 4))
		throw std::runtime_error("Failed to read file - 4 bytes");
	return (TWOBYTE_BIGENDIAN(buffer));
}

uint8_t*	image::readChunkData(std::fstream& file, uint32_t size)
{
	uint8_t*	buffer = new uint8_t[size];

	if (!file.read(reinterpret_cast<char*>(buffer), size))
	{
		delete[] buffer;
		throw std::runtime_error("Failed to read file - data");
	}
	return (buffer);
}

#undef TWOBYTE_BIGENDIAN

void	image::IHDR_Load(const uint8_t data[13])
{
	// Byte 0-3 -> width
	this->width = image::to4ByteUInt(&data[0]);
	// Byte 4-7 -> height
	this->height = image::to4ByteUInt(&data[4]);
	// Byte 9 -> colorType
	this->setColorType(ColorType(data[9]));
	// Byte 8 -> bitDepth
	this->setBitDepth(BitDepth(data[8]));
	// Byte 10 compression method, undefined
	// Byte 11 filter method, undefined
	// Byte 12 interlace method, undefined
}

uint8_t*		image::IDAT_UncompressData(uint8_t* data, unsigned long& size)
{
	uLongf		uSize = this->height * this->bytesPerRow();
	uint8_t*	uncompressed = new uint8_t[uSize];

	if (uncompress(uncompressed, &uSize, data, size) != Z_OK)
	{
		delete[] uncompressed;
		throw std::runtime_error("Failed to uncompress IDAT data");
	}
	size = uSize;
	return (uncompressed);
}


void	image::IDAT_Load(const uint8_t* data, unsigned long size)
{
	// Clear current map
	this->unsetMap();

	int				pixel = 0;
	rgba		color(0, 0, 0, 255);
	uint8_t			channel = 0;
	
	// Loop through data, 1 bit at a time
	//	For propper alignment, first loop per height row
	uint32_t	bitsPerRow = this->bytesPerRow() * 8;
	for (unsigned int y = 0; y < this->height; ++y)
	{
		uint16_t		value = 0;
		// starting position for the row: height + filter byte (8)
		unsigned int	rowStartingBit = y * bitsPerRow + 8;
		// Loop through width
		for (unsigned int bit = 0; bit < bitsPerRow; ++bit)
		{
			unsigned int pos = rowStartingBit + bit;
			value <<= 1;
			value |= (data[pos / 8] >> (7 - (pos % 8))) & 0b1;
			if ((bit + 1) % (unsigned int)this->bitDepth == 0)
			{
				this->IDAT_AddValueToRGBA(value, channel, color);
				value = 0;
				++channel;
				// If all channels are filled:
				//	set color in map, and set channel back to begin
				if (channel >= this->channels)
				{
					this->pixels[pixel % this->width][y] = color;
					++pixel;
					channel = 0;
				}
			}
		}
	}
}

void	image::IDAT_AddValueToRGBA(uint16_t value, uint8_t channel, rgba& color)
{
	uint8_t	ct = (uint8_t)this->colorType;

	value = this->convertColorBits(value, this->bitDepth, BitDepth::BIT_8);
	if (ct & 0b1)
		throw std::runtime_error("Color Type Palette unsupported");
	else
	{
		// Add value to the appropriate color attribute, depending on channel and color type
		switch (channel)
		{
			case 0:
				color.r = value;
				if ((ct & 0b10) == 0) // GRAYSCALE
					color.g = color.b = value;
				break;
			case 1:
				if ((ct & 0b10) == 0) // GRAYSCALE_ALPHA
					color.a = value;
				else
					color.g = value;
				break;
			case 2: // Only TRUECOLORs should reach this
				color.b = value;	break;
			case 3:	// ONLY TRUECOLOR_ALPHA should reach this
				color.a = value;	break;
			default:
				std::cerr	<< (int)ct	<< std::endl;
				std::cerr	<< "max channels "<< this->channels	<< std::endl;
				throw std::runtime_error("Invalid channel " + std::to_string(channel));
		}
	}
}

void	image::tEXt_Load(const uint8_t* data, unsigned long size)
{
	std::string		keyword;
	std::string		text;
	unsigned long	i = 0;

	for (; data[i] != 0 && i < size; ++i)
		keyword += data[i];
	++i;
	for (; i < size; ++i)
		text += data[i];
	this->metaData[keyword] = text;
}


#pragma endregion	"Load functions"

////////////////////////////////////////////////////////////////////////////////

#pragma region "Save functions"

bool	image::save(const char* name)
{
	try
	{
		image::File		fileClass(name);
		std::fstream&	fileStream = fileClass.open(std::fstream::out | std::fstream::trunc);

		this->createPrintableImage();
		this->saveToFile(fileStream);
	}
	catch(const std::exception& e)
	{
		std::cerr	<< "PNG Error";
		if (errno)
			std::cerr	<< '['	<< errno	<< strerror(errno)	<< ']';
		std::cerr	<< ": "	<< e.what()	<< std::endl;;
		return (false);
	}
	return (true);
}

void	image::createPrintableImage()
{
	// copy map
	this->printMap = this->pixels;

	//determine map's outer bounds
	int xLow = this->printMap.begin()->first;
	int	xHigh = this->printMap.rbegin()->first;
	int	yLow = this->printMap[xLow].begin()->first;
	int	yHigh = this->printMap[xLow].rbegin()->first;
	for (auto i = this->printMap.begin(); i != this->printMap.end(); ++i)
	{
		int newVal = i->second.begin()->first;
		yLow = newVal < yLow ? newVal : yLow;
		newVal = i->second.rbegin()->first;
		yHigh = newVal > yHigh ? newVal : yHigh;
	}
	// fill unset pixels with backgroundcolor
	for (int x = xLow; x <= xHigh; ++x)
	{
		for (int y = yLow; y <= yHigh; ++y)
		{
			if (this->printMap[x].find(y) == this->printMap[x].end())
				this->printMap[x][y] = rgba();
		}
	}

	// store size to class
	this->width = this->printMap.size();
	this->height = this->printMap.begin()->second.size();
}

void	image::saveToFile(std::fstream& file)
{
	// Critical
	this->Signature_Save(file);
	this->IHDR_Save(file);

	// Optional, before PLTE and IDAT
	if (false)
		this->cHRM_Save(file);
	if (false)
		this->gAMA_Save(file);
	if (false)
		this->iCCP_Save(file);
	if (false)
		this->sBIT_Save(file);
	if (false)
		this->sRGB_Save(file);
	
	// Critical if ColorType == PALETTE
	if (this->colorType == ColorType::PALETTE)
		this->PLTE_Save(file);
	
	// Optional, Before IDAT
	if (false)
		this->pHYs_Save(file);
	if (false)
		this->sPLT_Save(file);
	// Optional, After PLTE and before IDAT
	if (false)
		this->tRNS_Save(file);
	if (this->backgroundColor != nullptr)
		this->bKGD_Save(file);
	if (false)
		this->hIST_Save(file);

	// Critical
	this->IDAT_Save(file);

	// Optional
	if (false)
		this->tIME_Save(file);
	if (false)
		this->tEXt_Save(file);

	// Critical 
	this->IEND_Save(file);
}

void	image::Signature_Save(std::fstream& file)
{
	const unsigned char	signature[] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
	file.write(reinterpret_cast<const char*>(signature), sizeof(signature));
}

void	image::IHDR_Save(std::fstream& file)
{
	image::Chunk	chunk("IHDR");
	std::vector<uint8_t> data(13);

	this->IHDR_SetData(data);
	chunk.setData(data);
	chunk.writeTo(file);
}

void	image::IHDR_SetData(std::vector<uint8_t>& data)
{
	// width, chunkData byte 0-3
	image::to4ByteArray(this->width, &data[0]);
	//height, chunkData byte 4-7
	image::to4ByteArray(this->height, &data[4]);
	// Bit depth, chunkData byte 8
	data[8] = uint8_t(this->bitDepth);
	// Color type, chunkData byte 9
	data[9] = uint8_t(this->colorType);
	// Compression method, chunkData byte 10
	data[10] = 0;
	// Filter method, chunkData byte 11
	data[11] = 0;
	// Interlace method, chunkData byte 12
	data[12] = 0;
}

// Often only meaningfull for non-ALPHA images with tRNS set for general transperency.
void	image::bKGD_Save(std::fstream& file)
{
	image::Chunk	chunk("bKGD");

	std::vector<uint8_t>	data;
	switch (this->colorType)
	{
		case ColorType::PALETTE:
			data.resize(1, 0);
			break;
		case ColorType::GRAYSCALE:
		case ColorType::GRAYSCALE_ALPHA:
			data.resize(2, 0);
			this->setBits(data.data(), this->convertColorToGray(*this->backgroundColor), 0, int(BitDepth::BIT_16));
			break;
		case ColorType::TRUECOLOR:
		case ColorType::TRUECOLOR_ALPHA:
			data.resize(6, 0);
			this->setBits(data.data(), this->backgroundColor->r, 0, int(BitDepth::BIT_16));
			this->setBits(data.data(), this->backgroundColor->g, 16, int(BitDepth::BIT_16));
			this->setBits(data.data(), this->backgroundColor->b, 32, int(BitDepth::BIT_16));
			break;
		default:
			std::cerr	<< "Unknown ColorType"	<< std::endl;
			return;
	}
	chunk.writeTo(file);
}

void	image::IDAT_Save(std::fstream& file)
{
	image::Chunk			chunk("IDAT");
	std::vector<uint8_t>	data;
	std::vector<uint8_t>	compressed;

	data = this->IDAT_GenerateData();
	compressed = this->IDAT_CompressData(data);
	chunk.setData(compressed);
	chunk.writeTo(file);
}

std::vector<uint8_t>	image::IDAT_GenerateData()
{
	std::vector<uint8_t>	data;
	unsigned int			size = this->IDAT_CalculateSize();
	data.resize(size, 0);
	uint8_t*				buffer = data.data();
	short		bitDepth = short(this->bitDepth);

	unsigned	i = 0;
	for (auto y = this->printMap.begin()->second.begin(); y != this->printMap.begin()->second.end(); ++y)
	{
		// Clear padding from last row and Filter byte.
		this->setBits(buffer, 0, i, 8 + (8 - (i % 8)) % 8);
		// Allign bytes (+7) and move passed Filter byte (+8)
		i = ((i + 15) / 8) * 8;

		for (auto x = this->printMap.begin(); x != this->printMap.end(); ++x)
		{
			rgba&	pixel = this->printMap[x->first][y->first];
			if (this->colorType == ColorType::TRUECOLOR ||\
				this->colorType == ColorType::TRUECOLOR_ALPHA)
			{
				this->setBits(buffer, pixel.r, i, bitDepth);
				i += bitDepth;
				this->setBits(buffer, pixel.g, i, bitDepth);
				i += bitDepth;
				this->setBits(buffer, pixel.b, i, bitDepth);
				i += bitDepth;
			}
			else if (this->colorType == ColorType::GRAYSCALE ||\
					this->colorType == ColorType::GRAYSCALE_ALPHA)
			{
				this->setBits(buffer, this->convertColorToGray(pixel), i, bitDepth);
				i += bitDepth;
			}

			if (this->colorType == ColorType::GRAYSCALE_ALPHA ||\
				this->colorType == ColorType::TRUECOLOR_ALPHA)
			{
				this->setBits(buffer, pixel.a, i, bitDepth);
				i += bitDepth;
			}
		}
	}
	return (data);
}

unsigned int	image::IDAT_CalculateSize()
{
	unsigned	width = this->printMap.size();
	unsigned	height = this->printMap.begin()->second.size();

	// Bits per channel
	unsigned	bitsPerPixel = int(this->bitDepth);
	// Multiply by channels (amount of ' arguments')
	switch (this->colorType)
	{
		case ColorType::GRAYSCALE:			bitsPerPixel *= 1;	break;
		case ColorType::TRUECOLOR:			bitsPerPixel *= 3;	break;
		case ColorType::PALETTE:			bitsPerPixel *= 1;	break;
		case ColorType::GRAYSCALE_ALPHA:	bitsPerPixel *= 2;	break;
		case ColorType::TRUECOLOR_ALPHA:	bitsPerPixel *= 4;	break;
		default:							bitsPerPixel *= 4;	break;
	}
	// Filter byte + width. Ensure multiple of 8, for byte allignment per row.
	unsigned	bytesPerRow = 1 + (width * bitsPerPixel + 7) / 8;
	return (bytesPerRow * height);
}

std::vector<uint8_t>	image::IDAT_CompressData(std::vector<uint8_t>& data)
{
	std::vector<uint8_t>	compressed;
	uLong					size = compressBound(data.size());

	compressed.resize(size);
	switch (compress(compressed.data(), &size, data.data(), static_cast<uLong>(data.size())))
	{
		case Z_OK:			break;
		case Z_MEM_ERROR:	throw std::runtime_error("Compression failure (memory)");
		case Z_BUF_ERROR:	throw std::runtime_error("Compression failure (buffer)");
		default:			throw std::runtime_error("Compression failure");
	}
	compressed.resize(size);
	return (compressed);
}

#ifdef DEBUG
# include <iomanip>

void	image::printBuffer(const uint8_t* buffer, unsigned int size)
{
	unsigned short line = 1 + (this->printMap.size() * int(this->bitDepth) + 7) / 8;
	for (unsigned int i = 0; i < size; ++i)
	{
		for (short bit = 7; bit >= 0; --bit)
		{
			std::cout	<< ((buffer[i] >> bit) & 1);
		}
		if (i % line == line - 1)
			std::cout	<< std::endl;
		else
			std::cout	<< ' ';
	}
}
#endif

void	image::tIME_Save(std::fstream& file)
{
	image::Chunk			chunk("tIME");
	std::vector<uint8_t>	data(7);
	tm						utc = getUTC();

	data[0] = (utc.tm_year + 1900) >> 8;
	data[1] = (utc.tm_year + 1900) & 0xFF;
	data[2] = utc.tm_mon + 1;
	data[3] = utc.tm_mday;
	data[4] = utc.tm_hour;
	data[5] = utc.tm_min;
	data[6] = utc.tm_sec;

	chunk.setData(data);
	chunk.writeTo(file);
}

void	image::tEXt_Save(std::fstream& file)
{
	for (auto& meta : this->metaData)
		this->tEXt_SaveOneChunk(file, meta.first, meta.second);
	this->tEXt_SaveEmptyKeywords(file);
}

void	image::tEXt_SaveOneChunk(std::fstream& file, std::string keyword, std::string text)
{
	image::Chunk			chunk("tEXt");
	std::vector<uint8_t>	data;
	std::size_t				size(keyword.size() + text.size() + 1);

	data.reserve(size);
	data.insert(data.end(), keyword.begin(), keyword.end());
	data.push_back(0);
	// Optionally add compression if more than 512 bytes
	if (size <= 512)
		data.insert(data.end(), text.begin(), text.end());
	else
	{
		std::vector<uint8_t>	vText(text.begin(), text.end());
		uLongf	cSize = compressBound(vText.size());
		std::vector<uint8_t>	cText(cSize);
		if (compress(cText.data(), &cSize, vText.data(), vText.size()) == Z_OK)
		{
			chunk.setType("zTXt");
			data.push_back(0); // Compression method
			cText.resize(cSize);
			data.insert(data.end(), cText.begin(), cText.end());
		}
		else
			data.insert(data.end(), text.begin(), text.end());
	}
	chunk.setData(data);
	chunk.writeTo(file);
}

void	image::tEXt_SaveEmptyKeywords(std::fstream& file)
{
	// Creation Time
	if (this->metaData.find(MetaData::CreationTime) == this->metaData.end())
	{
		char		buffer[64];
		std::tm	utc = getUTC();

		std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z", &utc);
		this->tEXt_SaveOneChunk(file, MetaData::CreationTime, buffer);
	}
	// Source
	if (this->metaData.find(MetaData::Source) == this->metaData.end())
	{
		utsname	buffer;
		uname(&buffer);
		this->tEXt_SaveOneChunk(file, MetaData::Source, std::string(buffer.sysname) + '-' + buffer.release);
	}
	// Software
	if (this->metaData.find(MetaData::Software) == this->metaData.end())
	{
		this->tEXt_SaveOneChunk(file, MetaData::Software, SOFTWARENAME);
		this->tEXt_SaveOneChunk(file, "Website", WEBSITEURL);
	}
}

void	image::IEND_Save(std::fstream& file)
{
	image::Chunk	chunk("IEND");

	chunk.writeTo(file);
}

unsigned short	image::convertColorBits(unsigned short value, BitDepth from, BitDepth to)
{
	if (from == to)
		return (value);
	short	f = short(from);
	short	t = short(to);
	if (from > to)
		return (value >> (f - t));
	value = value << (t - f);
	return (value | (value >> f));
}

unsigned short	image::convertColorToGray(const rgba col)
{
	return (0.299 * col.r + 0.587 * col.g + 0.114 * col.b);
}

#pragma endregion	"Save functions"

////////////////////////////////////////////////////////////////////////////////

#pragma region	"Helper functions"

uint8_t	image::bitsPerPixel()
{
	return ((uint8_t)this->bitDepth * this->channels);
}

uint32_t	image::bytesPerRow()
{
	return (1 + (this->width * this->bitsPerPixel() + 7) / 8);
}


// chatgpt
void	image::setBits(uint8_t* ptr, int value, int bitIndex, int bitCount)
{
	int byteIndex = bitIndex / 8;
	int bitOffset = bitIndex % 8;

	value = this->convertColorBits(value, BitDepth::BIT_8, this->bitDepth);

	while (bitCount > 0)
	{
		// How many bits we can write in the current byte
		int bitsInThisByte = std::min(8 - bitOffset, bitCount);

		// For PNG, bits are stored MSB first within a byte
		int shift = 8 - bitOffset - bitsInThisByte;

		// Create mask for bits we're going to replace
		uint8_t mask = ((1 << bitsInThisByte) - 1) << shift;

		// Clear bits and insert the correct bits from value (take the MSB portion)
		ptr[byteIndex] = (ptr[byteIndex] & ~mask) | ((value >> (bitCount - bitsInThisByte)) << shift & mask);

		bitCount -= bitsInThisByte;
		byteIndex++;
		bitOffset = 0;
	}
}


#pragma endregion	"Helper functions"

}

////////////////////////////////////////////////////////////////////////////////
