#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace git_pilot {
namespace tools {

struct ToolParameter {
	std::string    name;
	std::string    type;
	std::string    description;
	bool           required = false;
	nlohmann::json schema;
};

struct ToolDefinition {
	std::string                name;
	std::string                description;
	std::vector<ToolParameter> parameters;

	nlohmann::json to_json_schema() const
	{
		nlohmann::json schema;
		schema["type"]       = "object";
		schema["properties"] = nlohmann::json::object();
		schema["required"]   = nlohmann::json::array();

		for (const auto &param : parameters) {
			schema["properties"][param.name] = param.schema;
			if (param.required) {
				schema["required"].push_back(param.name);
			}
		}

		return schema;
	}
};

class ToolBase
{
public:
	virtual ~ToolBase() = default;
	virtual ToolDefinition get_definition() const                   = 0;
	virtual nlohmann::json execute(const nlohmann::json &arguments) = 0;
	virtual bool validate_arguments(const nlohmann::json &args) const { return true; }
};

}  // namespace tools
}  // namespace git_pilot
