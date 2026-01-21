#include "../include/pngimg.hpp"

#include <iostream>	//std::cout
# include <zlib.h>	// crc32(uLongf)

namespace
{

uint32_t	swapEndian(uint32_t value)
{
	return (((value >> 24) & 0xFF) |
			((value >> 8)  & 0xFF00) |
			((value << 8)  & 0xFF0000) |
			((value << 24) & 0xFF000000));
}

}

namespace png
{

void	image::Chunk::readFrom(std::fstream& file)
{
	uint32_t	size = readFrom_4Bytes(file);
	file.read(this->type, 4);
	this->data = readFrom_NBytes(file, size);
	if (readFrom_4Bytes(file) != generateCRC(this->type, this->data))
		throw std::runtime_error("CRC did not match, chunk corrupted");
	
	std::cout << "Read: "	<< std::string(this->type, 4)	<< std::endl;
}

uint32_t	image::Chunk::readFrom_4Bytes(std::fstream& file)
{
	std::vector<uint8_t>	bytes = readFrom_NBytes(file, 4);

	return (uint32_t(bytes[0]) << 24 |
			uint32_t(bytes[1]) << 16 |
			uint32_t(bytes[2]) << 8 |
			uint32_t(bytes[3]));
}

std::vector<uint8_t>	image::Chunk::readFrom_NBytes(std::fstream& file, uint32_t size)
{
	std::vector<uint8_t> bytes(size);

	file.read(reinterpret_cast<char*>(bytes.data()), size);
	if (!file)
		throw std::runtime_error(std::string("Failed to read ") + std::to_string(size) + " bytes for chunk");
	return (bytes);
}

void	image::Chunk::writeTo(std::fstream& file) const
{
	// std::cout	<< "Saving "	<< std::string(this->type, 4)	<< std::endl;
	Chunk::writeUint32To(file, this->data.size());
	file.write(this->type, 4);
	file.write(reinterpret_cast<const char*>(this->data.data()), this->data.size());
	Chunk::writeUint32To(file, Chunk::generateCRC(this->type, this->data));
}

void	image::Chunk::writeUint32To(std::fstream& file, uint32_t val)
{
	// Ensure bits are in the right order
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	val = swapEndian(val);
#endif
	file.write(reinterpret_cast<const char*>(&val), 4);
}

uint32_t	image::Chunk::generateCRC(const char type[4], const std::vector<uint8_t>& data)
{
	uint32_t	crc;

	// Generate CRC
	crc = crc32(0, NULL, 0);
	crc = crc32(crc, (Bytef*)type, 4);
	if (data.size() > 0)
		crc = crc32(crc, data.data(), data.size());

	// Ensure bits are in the right order
// #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
// 	crc = swapEndian(crc);
// #endif
	return (crc);
}

void		image::Chunk::setType(uint32_t type)
{
	char	typeArray[4];

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	type = swapEndian(type);
#endif
	for (int i = 0; i < 4; ++i)
		typeArray[i] = uint8_t(type >> (24 - 8 * i));
	this->setType(typeArray);
}

void		image::Chunk::setType(const char type[4])
{
	for (int i = 0; i < 4; ++i)
	{
		if ((type[i] < 'A' || (type[i] > 'Z' && type[i] < 'a') || type[i] > 'z') ||
			(i == 2 && type[i] >'Z'))
			throw std::runtime_error(std::string("Incorrect character for chunk Typename[") + std::to_string(i) + "]: " + type[i]);
		this->type[i] = type[i];
	}
}

}
