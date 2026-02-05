#include "../include/pngimg.hpp"

#include <unistd.h>	// access
// #include <stdlib.h>	// getenv
#include <limits.h>	// PATH_MAX
#include <iostream> // std::cerr


namespace png
{

image::File::File(const char* path)
{
	this->path = path;

	std::size_t	pos = this->path.find_last_of('/');
	std::string dir = this->generatePath(pos);
	std::string	name = this->generateName(pos);
	this->path = dir + '/' + name;
	this->validatePath();
}

image::File::~File()
{
	this->file.close();
}

std::string	image::File::generatePath(std::size_t pos)
{
	std::string	path;

	if (pos != std::string::npos)
	{
		path = this->path.substr(0, pos);
		if (path[0] == '~')
		{
			const char*	home = getenv("HOME");
			if (!home)
				throw std::runtime_error("Failed to convert ~ in file path");
			path = std::string(home) + path.substr(1);
		}
	}
	else
		path = "./";

	char rp[PATH_MAX];
	if (!realpath(path.c_str(), rp))
		throw std::invalid_argument("Failed to create realpath");
	return (rp);
}

std::string	image::File::generateName(std::size_t pos)
{
	std::string	name;

	if (pos != std::string::npos)
		name = this->path.substr(pos + 1);
	else
		name = this->path;

	if (!this-> nameHasValidExtension(name, ".png"))
	{
		std::cerr	<< "Improper extension. Adding .png"	<< std::endl;
		name.append(".png");
	}
	return (name);
}

bool	image::File::nameHasValidExtension(std::string name, const std::string extension)
{
	if (name.size() <= extension.size() + 1)
		return (false);

	std::size_t	pos = name.find_last_of('.');
	if (pos == std::string::npos)
		return (false);

	name = name.substr(pos);
	for (std::size_t i = 1; i < name.size(); ++i)
		name[i] = std::tolower(name[i]);
	return (name == extension);
}

void	image::File::validatePath()
{
	const char*	checkName = this->path.c_str();

	if (!access(checkName, F_OK) && access(checkName, R_OK) && access(checkName, W_OK))
		throw std::invalid_argument(std::string("File ").append(checkName).append(" already exists, but is not usable."));
}

std::fstream&	image::File::open(std::ios_base::openmode __mode)
{
	this->file.open(this->path, __mode);
	return (this->getFile());
}

std::fstream&	image::File::getFile()
{
	if (!this->file.is_open())
		throw std::runtime_error("Couldn't open " + this->path);
	return (this->file);
}

}
