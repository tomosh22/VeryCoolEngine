/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/

Credit Rich Davison
*/
#include "Assets.h"
#include "vcePCH.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace VeryCoolEngine {
	bool ReadTextFile(const std::string& filepath, std::string& result) {
		std::ifstream file(filepath, std::ios::in);
		if (file) {
			std::ostringstream stream;

			stream << file.rdbuf();

			result = stream.str();

			return true;
		}
		else {
			std::cout << __FUNCTION__ << " can't read file " << filepath << "\n";
			return false;
		}
	}

	bool	ReadBinaryFile(const std::string& filename, char** into, size_t& size) {
		std::ifstream file(filename, std::ios::binary);

		if (!file) {
			return false;
		}

		file.seekg(0, std::ios_base::end);

		std::streamoff filesize = file.tellg();

		file.seekg(0, std::ios_base::beg);

		char* data = new char[(unsigned int)filesize];

		file.read(data, filesize);

		file.close();

		*into = data;
		size = filesize;

		return data == NULL ? true : false;
	}
}

