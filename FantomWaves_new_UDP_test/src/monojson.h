#ifndef monojson_h_
#define monojson_h_

#include <string>
#include <sstream>
#include "picojson.h"
#include <FantomWaves.h>

namespace local
{
	class monojson
	{
	public:
		bool load(const char * path);	// true...succeeded, false...failed
		bool get_bool(const char * name);
		std::string get_string(const char * name);

		template<typename T> T get_number(const char * name)
		{
			return static_cast<T>(root()[name].get<double>());
		}



	private:
		fw::ifstream stream;
		picojson::value json_data;

		picojson::object & root();
	};
}

#endif
