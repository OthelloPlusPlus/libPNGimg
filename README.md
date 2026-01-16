`png::image` is a **C++ class** for creating, reading, and writing PNG image files. It provides direct RGBA pixel access and basic image manipulation with <u>minimal external dependencies</u>.

# Supported Platforms
- Linux

# Dependency
- **zlib** is required to support **cyclic redundancy check (CRC)** operations used by the PNG format for checksum generation and validation.

# Installation
The `configure` script generates a `Makefile`. The installation prefix specifies the destination directory and defaults to `/usr/local`.
```sh
./configure --prefix=/usr/local
```

Build the static library:
```sh
make
```
This produces the `libpngimg.a` archive.

To install the library and header files:
```sh
make install
```
Installation deploys:
- `${prefix}/lib/libpngimg.a`
- `${prefix}/include/pngimg.hpp`

After installation, you can run the test suite, which generates sample images in the `test/` directory:
```sh
make test
```
# Implementation
`png::image` supports reading from existing files and creating new images. Pixel access is performed via `operator[]`, providing row-based access to pixels in RGBA form with channel values in the range **0–255**. Both 8-bit and 16-bit bit depths are supported; conversion behavior depends on configured bit depth.

Operations may fail (e.g., loading or saving files) and will print error messages to the terminal. Users should check for success where appropriate.

```c++
#include <pngimg.hpp>

void	createFile()
{
	png::image	img;

	// Configure File
	img.setColorType(png::ColorType::TRUECOLOR_ALPHA);
	img.setBitDepth(png::BitDepth::BIT_16);
	// Set a square of 10x10 pixels to orange
	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
			img[x][y] = png::rgba(255, 127, 0, 255);
	// Save the file
	if (img.save("/path/to/newfile.png"))
		std::cout	<< "Saved the file"	<< std::endl;
}

void	adjustFile()
{
	png::image	img;

	// Load the file
	if (!img.load("/path/to/file.png"))
		return ;
	// Set a square of 10x10 pixels to orange
	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
			img[x][y] = png::rgba(255, 127, 0, 255);
	// Save the file
	if (img.save("/path/to/file.png"))
		std::cout	<< "Saved the file"	<< std::endl;
}
```

# Compilation
Compile and link against the PNG library and zlib:
```sh
g++ main.cpp -lpngimg -lz
```

If the library is not installed in a default system location, specify the include and library search paths explicitly:
```sh
g++ main.cpp -I${prefix}/include -L${prefix}/lib -lpngimg -lz
```

# Technical Information

## Chunk Storage
A PNG file starts with a default sginature `{0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'}` followed by a sequence of data chunks. A few chunks are critical (IHDR IDAT IEND), whereas the rest is optional, and many chunks have a specific order in which they appear.
<table>
	<thead>
		<tr>
			<th>Bytes</th>
			<th>Field</th>
			<th>Description</th>
		</tr>
	</thead>
	<tbody>
		<tr>
			<td>4</td>
			<td>Chunk Size</td>
			<td>Length of the chunk data in bytes (big-endian)</td>
		</tr>
		<tr>
			<td>4</td>
			<td>Chunk Type</td>
			<td>4-letter name of the chunk type (e.g., IHDR, IDAT, IEND)</td>
		</tr>
		<tr>
			<td>n</td>
			<td>Chunk Data</td>
			<td>The actual data of the chunk.<br>Its length is specified in Chunk Size is thereby capped at 2^32 - 1 bytes (4,294,967,295)</td>
		</tr>
		<tr>
			<td>4</td>
			<td>CRC</td>
			<td>Cyclic redundancy check for the chunk type and data, used to ensure data integrity</td>
		</tr>
	</tbody>
</table>

## Channel Mapping
The `IHDR` chunk specifies the **Color Type** and **Bit Depth** of the PNG image. 

**Color Type** determines how many channels each pixel uses and what those channels represent. It can specify black-and-white (GRAYSCALE) or full-color (TRUECOLOR) images, with the option to include a transparency channel (ALPHA). Alternatively, a palette-based (PALETTE) color type uses a single channel whose values index into the `PLTE` chunk.

For converting a color image to grayscale, the luminance Y is calculated as a weighted average of RGB: $$Y = 0.299 R + 0.587 G + 0.114 B$$

| Channel ↓ \ ColorType → | GRAYSCALE | GRAYSCALE_ALPHA | TRUECOLOR | TRUECOLOR_ALPHA |
| :---------------------- | :-------- | :-------------- | :-------- | :-------------- |
| 0                       | Y (R,G,B) | Y (R,G,B)       | R         | R               |
| 1                       | —         | A               | G         | G               |
| 2                       | —         | —               | B         | B               |
| 3                       | —         | —               | —         | A               |

**Bit Depth** dictates the number of bits per channel, defining the range of values each channel can store. The most common depth, 8 bits (1 byte), allows the standard RGB range of 0–255.

# Sources
https://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html

https://medium.com/@0xwan/png-structure-for-beginner-8363ce2a9f73

https://www.w3.org/TR/PNG-Chunks.html

## read metadata
https://framebird.io/exif-metadata-viewer/png

https://pics.io/photo-metadata-viewer

# Creator(s)
Othello<br>
[<img alt="LinkedIn" height="32px" src="https://github.githubassets.com/images/modules/logos_page/GitHub-Mark.png" target="_blank" />](https://github.com/OthelloPlusPlus)
[<img alt="LinkedIn" height="32px" src="https://upload.wikimedia.org/wikipedia/commons/thumb/c/ca/LinkedIn_logo_initials.png/600px-LinkedIn_logo_initials.png" target="_blank" />](https://nl.linkedin.com/in/orlando-hengelmolen)

# License
Copyright (c) 2026 Othello

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

1. The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

2. Any use of the Software in a public project, website, application, or distributed
   product must include clear attribution to the author (Othello), such as in a
   credits file, documentation, or an “About” section.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.




# Developer Notes
## Read file in terminal
```sh
xxd image.png
```


## Install
custom install
```sh
make clean && ./configure --prefix=$HOME/.local && make && make install && make test
```


## notes
```sh
echo "$PATH" | sed -e 's/:/:\n/g' -e "s#$HOME#\$HOME#g" | sort
```

```sh
make clean && ./configure --prefix=$HOME/.local && make && make install
./configure --prefix=$HOME/.local
make
make install
```

```sh
c++ -v -E - < /dev/null 2>&1 | sed -n '/#include <...> search starts here:/,/End of search list./p'
```

```sh
g++ -print-search-dirs | sed -e 's/:/:\n/g' | sort
```
