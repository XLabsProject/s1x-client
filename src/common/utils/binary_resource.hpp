#pragma once

#include <string>

namespace utils
{
	class binary_resource
	{
	public:
		binary_resource(int id, std::string file);

		std::string get_extracted_file();

	private:
		std::string resource_;
		std::string filename_;
		std::string path_;
	};
}
