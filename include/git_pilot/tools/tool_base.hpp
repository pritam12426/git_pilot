#pragma once

#include <boost/json.hpp>

#include <memory>
#include <string>
#include <vector>

namespace git_pilot::tools {

struct ToolParameter {
	std::string      name;
	std::string      type;
	std::string      description;
	bool             required = false;
	boost::json::value schema;
};

struct ToolDefinition {
	std::string                 name;
	std::string                 description;
	std::vector<ToolParameter>  parameters;

	[[nodiscard]] boost::json::value to_json_schema() const
	{
		boost::json::object schema;
		schema["type"]       = "object";
		schema["properties"] = boost::json::object{};
		schema["required"]   = boost::json::array{};

		for (const auto &param : parameters) {
			schema["properties"].as_object()[param.name] = param.schema;
			if (param.required) {
				schema["required"].as_array().push_back(
				    boost::json::value(param.name));
			}
		}

		return schema;
	}
};

class ToolBase
{
public:
	virtual ~ToolBase() = default;
	virtual ToolDefinition get_definition() const                         = 0;
	virtual boost::json::value execute(const boost::json::value &arguments) = 0;
	virtual bool validate_arguments(const boost::json::value &args) const { return true; }
};

}  // namespace git_pilot::tools
