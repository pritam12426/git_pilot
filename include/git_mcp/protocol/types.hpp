#pragma once

#include <boost/json.hpp>

#include <cstdint>
#include <optional>
#include <string>

namespace git_mcp::protocol {

struct JsonRpcRequest {
	std::string    method;
	boost::json::value params;
	boost::json::value id;
};

struct JsonRpcError {
	int                    code;
	std::string            message;
	std::optional<boost::json::value> data;
};

struct JsonRpcResponse {
	boost::json::value                id;
	std::optional<boost::json::value> result;
	std::optional<JsonRpcError>       error;
};

[[nodiscard]] auto parse_request(const boost::json::value &msg) -> std::optional<JsonRpcRequest>;

[[nodiscard]] auto make_response(const JsonRpcResponse &res) -> boost::json::value;

[[nodiscard]] auto make_error_response(int code, const std::string &message,
                                       boost::json::value id = nullptr) -> boost::json::value;

[[nodiscard]] auto make_success_response(boost::json::value id,
                                         boost::json::value result) -> boost::json::value;

} // namespace git_mcp::protocol
