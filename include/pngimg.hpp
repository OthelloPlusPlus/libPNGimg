/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pngimg.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ohengelm <ohengelm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/28 14:24:34 by ohengelm          #+#    #+#             */
/*   Updated: 2026/01/21 21:04:09 by ohengelm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PNGIMG_HPP
# define PNGIMG_HPP

# include <fstream>	// std::fstream
# include <map>	// std::map
# include <vector>	// std::vector
# include <queue>	// std::queue
// # include <zlib.h>	// crc32(uLongf)

namespace png
{
	struct rgba
	{
		short	r;
		short	g;
		short	b;
		short	a;

		rgba() : r(0), g(0), b(0), a(0) {}
		rgba(short r, short g, short b, short a = 255): r(r), g(g), b(b), a(a) {}
	};

	enum class BitDepth : uint8_t
	{
		BIT_1	= 1,	// 0-1
		BIT_2	= 2,	// 0-3
		BIT_4	= 4,	// 0-15
		BIT_8	= 8,	// 0-255
		BIT_16	= 16	// 0-65535
	};

	enum class ColorType : uint8_t
	{
		GRAYSCALE		= 0,	// 0000
		TRUECOLOR		= 2,	// 0010
		PALETTE			= 3,	// 0011
		GRAYSCALE_ALPHA	= 4,	// 0100
		TRUECOLOR_ALPHA	= 6,	// 0110
	};

	namespace MetaData
	{
		extern const char* Title;
		extern const char* Author;
		extern const char* Description;
		extern const char* Copyright;
		extern const char* CreationTime;
		extern const char* Software;
		extern const char* Disclaimer;
		extern const char* Warning;
		extern const char* Source;
		extern const char* Comment;
	};

	struct MetaKey
	{
		const char* key;
		MetaKey(const char* k) : key(k) {}
	};


class image
{
	public:
		image();
		image(const char* fileToLoad);
		~image();

		void	setBitDepth(png::BitDepth depth);
		void	setColorType(png::ColorType	type);
		void	setBackgroundColor(rgba color);
		void	setBackgroundColor(short r, short g, short b);
		void	setBackgroundColor(short r, short g, short b, short a);
		void	unsetBackgroundColor();
		void	unsetMap();

		void	addMetaData(png::MetaKey keyword, const char* text);
		void	addMetaData(const char* keyword, const char* text);
		void	rmMetaData(png::MetaKey keyword);
		void	rmMetaData(const char* keyword);
		void	clearMetaData();

		std::map<int, rgba>&	operator[](int x) {return this->pixels[x];}
		void	drawLine(int xStart, int yStart, int xEnd, int yEnd, rgba color);
		void	drawLine(const std::vector<std::pair<int, int>> line, rgba color);
		void	drawShape(const std::vector<std::pair<int, int>> shape, rgba color);
		void	drawShape(const std::vector<std::pair<int, int>> shape, rgba color, rgba fill);
		void	drawEllipse(int xBegin, int yBegin, int xEnd, int yEnd, rgba color);
		void	drawEllipse(double xCenter, double xRadius, double yCenter, double yRadius, rgba color);

		bool	save(const char* name);
		bool	load(const char* name);

	private:
		std::string	name;
		rgba*		backgroundColor = nullptr;
		ColorType	colorType;
		BitDepth	bitDepth;
		uint8_t		channels;
		uint32_t	width, height;

		std::map<std::string, std::string>	metaData;

		std::map<int, std::map<int, rgba>>	pixels;
		std::map<int, std::map<int, rgba>>	printMap;

		// std::queue<int, int>	line;
		// rgba					lineColor;
		// std::queue<int, int>	shape;
		// rgba					shapeBorderColor;
		// rgba					shapeContentColor;

		void	calculateChannels();

		void	saveToFile(std::fstream& file);
		void	createPrintableImage();

		bool		readChunk(std::fstream& file);
		static uint32_t	readChunk4Bytes(std::fstream& file);
		static uint8_t*	readChunkData(std::fstream& file, uint32_t size);

		void		Signature_Save(std::fstream& file);
		static bool	Signature_Confirm(std::fstream& file);

		void	IHDR_Save(std::fstream& file);
		void		IHDR_SetData(std::vector<uint8_t>& data);
		void	IHDR_Load(const uint8_t data[13]);

		void	cHRM_Save(std::fstream& file) {};
		void	gAMA_Save(std::fstream& file) {};
		void	iCCP_Save(std::fstream& file) {};
		void	sBIT_Save(std::fstream& file) {};
		void	sRGB_Save(std::fstream& file) {};
		void	PLTE_Save(std::fstream& file) {};
		void	pHYs_Save(std::fstream& file) {};
		void	sPLT_Save(std::fstream& file) {};
		void	tRNS_Save(std::fstream& file) {};

		void	bKGD_Save(std::fstream& file);

		void	hIST_Save(std::fstream& file) {};


		void	IDAT_Save(std::fstream& file);
		std::vector<uint8_t>	IDAT_GenerateData();
		unsigned int			IDAT_CalculateSize();
		std::vector<uint8_t>	IDAT_CompressData(std::vector<uint8_t>& data);
		void	IDAT_Load(const uint8_t* data, unsigned long size);
		uint8_t*	IDAT_UncompressData(uint8_t* data, unsigned long& size);
		void		IDAT_AddValueToRGBA(uint16_t value, uint8_t channel, png::rgba& color);

		void	tIME_Save(std::fstream& file);

		void	tEXt_Save(std::fstream& file);
		void		tEXt_SaveOneChunk(std::fstream &file, std::string keyword, std::string text);
		void		tEXt_SaveEmptyKeywords(std::fstream& file);
		void	tEXt_Load(const uint8_t* data, unsigned long size);

		void	IEND_Save(std::fstream& file);

		unsigned short	convertColorBits(unsigned short value, png::BitDepth from, png::BitDepth to);
		unsigned short	convertColorToGray(const rgba col);
// #ifdef DEBUG
		void	printBuffer(const uint8_t* buffer, unsigned int size);
		void	printPixelMap();
		void	printMetaData();
// #endif
		uint8_t		bitsPerPixel();
		uint32_t	bytesPerRow();
		void		setBits(uint8_t* ptr, int value, int bitIndex, int bitCount);

		class Chunk
		{
			public:
				Chunk(){};
				Chunk(const char typeName[4]){this->setType(typeName);};
				~Chunk(){}; // WIP

				void		writeTo(std::fstream& file) const;
				static void		writeUint32To(std::fstream& file, uint32_t val);

				void						readFrom(std::fstream& file);
				static uint32_t					readFrom_4Bytes(std::fstream& file);
				static std::vector<uint8_t>		readFrom_NBytes(std::fstream& file, uint32_t size);

				static uint32_t	generateCRC(const char type[4], const std::vector<uint8_t>& data);

				std::size_t	getSize() const {return this->data.size();}
				void		setType(const char type[4]);
				void		setType(uint32_t type);
				const char*	getType() const {return this->type;}
				void		setData(const std::vector<uint8_t>& data) {this->data = data;};
				std::vector<uint8_t>	getData() const {return this->data;}

			private:
				char					type[4];
				std::vector<uint8_t>	data;
		};
		class File
		{
			public:
				File(const char* path);
				~File();

				std::fstream&	open(std::ios_base::openmode __mode);
				std::fstream&	getFile();
			
				private:
					std::fstream	file;
					std::string		path;

					std::string	generatePath(std::size_t pos);
					std::string	generateName(std::size_t pos);
					bool		nameHasValidExtension(std::string name, const std::string extension);
					void		validatePath();
		};

		template<typename SrcT, typename DstT>
		static void	to4ByteArray(SrcT src, DstT dst[4])
		{
			static_assert(std::is_integral<SrcT>::value, "to4byteArray src expects an integer type");
			static_assert(std::is_integral<DstT>::value && sizeof(DstT) == 1, "to4ByteArray dst expects a 1-byte integral type");

			uint32_t	value = static_cast<uint32_t>(src);
			dst[0] = (value >> 24) & 0xFF;
			dst[1] = (value >> 16) & 0xFF;
			dst[2] = (value >> 8) & 0xFF;
			dst[3] = value & 0xFF;
		}

		template<typename T>
		static uint32_t	to4ByteUInt(const T src[4])
		{
			static_assert(std::is_integral<T>::value && sizeof(T) == 1, "to4ByteUInt src expects a 1-byte integral type");

			return ((uint32_t(src[0]) << 24) | 
					(uint32_t(src[1]) << 16) | 
					(uint32_t(src[2]) << 8) | 
					uint32_t(src[3]));
		}

};

}

#endif
