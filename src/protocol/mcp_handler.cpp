#include "git_mcp/protocol/mcp_handler.hpp"
#include "git_mcp/protocol/types.hpp"

#include "git_pilot/tools/tool_registry.hpp"
#include "git_pilot/utils/logger.hpp"

#include <exception>
#include <string>

namespace git_mcp::protocol {

using namespace git_pilot::utils;

auto handle_list_tools(boost::json::value id) -> boost::json::value
{
	auto &registry = git_pilot::tools::ToolRegistry::instance();
	auto defs = registry.all_definitions();

	boost::json::array tools_arr;
	for (const auto &def : defs) {
		boost::json::object tool;
		tool["name"]        = def.name;
		tool["description"] = def.description;
		tool["inputSchema"] = def.to_json_schema();
		tools_arr.push_back(std::move(tool));
	}

	boost::json::object result;
	result["tools"] = std::move(tools_arr);
	return make_success_response(std::move(id), std::move(result));
}

auto handle_call_tool(const boost::json::value &params, boost::json::value id) -> boost::json::value
{
	if (!params.is_object()) {
		return make_error_response(-32602, "Invalid params", std::move(id));
	}

	auto const &pobj = params.as_object();

	auto it_name = pobj.find("name");
	if (it_name == pobj.end() || !it_name->value().is_string()) {
		return make_error_response(-32602, "Missing tool name", std::move(id));
	}
	auto tool_name = boost::json::value_to<std::string>(it_name->value());

	auto &registry = git_pilot::tools::ToolRegistry::instance();
	auto *tool = registry.find(tool_name);
	if (!tool) {
		return make_error_response(-32602,
		    "Unknown tool: " + tool_name, std::move(id));
	}

	boost::json::value arguments;
	auto it_args = pobj.find("arguments");
	if (it_args != pobj.end() && it_args->value().is_object()) {
		arguments = it_args->value();
	}

	try {
		auto result = tool->execute(arguments);
		boost::json::object content_item;
		content_item["type"] = "text";
		content_item["text"] = boost::json::serialize(result);
		boost::json::array content;
		content.push_back(std::move(content_item));

		boost::json::object response;
		response["content"] = std::move(content);
		return make_success_response(std::move(id), std::move(response));
	} catch (const std::exception &e) {
		LOG_ERROR("Tool '{}' execution failed: {}", tool_name, e.what());
		return make_error_response(-32603,
		    "Tool execution failed: " + std::string(e.what()), std::move(id));
	}
}

auto handle_mcp_message(const boost::json::value &request) -> boost::json::value
{
	auto req = parse_request(request);
	if (!req.has_value()) {
		return make_error_response(-32600, "Invalid Request", nullptr);
	}

	if (req->method == "tools/list") {
		return handle_list_tools(std::move(req->id));
	} else if (req->method == "tools/call") {
		return handle_call_tool(req->params, std::move(req->id));
	} else {
		return make_error_response(-32601,
		    "Method not found: " + req->method, std::move(req->id));
	}
}

} // namespace git_mcp::protocol
