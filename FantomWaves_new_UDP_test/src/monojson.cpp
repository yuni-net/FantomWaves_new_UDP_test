#include "monojson.h"

namespace local
{
	bool monojson::load(const char * path)
	{
		stream.read(path);
		picojson::parse(json_data, stream.stream());

		return true;
	}

	bool monojson::get_bool(const char * name)
	{
		return root()[name].get<bool>();
	}

	std::string monojson::get_string(const char * name)
	{
		return root()[name].get<std::string>();
	}





	picojson::object & monojson::root()
	{
		return json_data.get<picojson::object>();
	}

}
