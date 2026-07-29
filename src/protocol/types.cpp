#include "git_mcp/protocol/types.hpp"

namespace git_mcp::protocol {

auto parse_request(const boost::json::value &msg) -> std::optional<JsonRpcRequest>
{
	if (!msg.is_object()) {
		return std::nullopt;
	}

	auto const &obj = msg.as_object();

	auto it_jsonrpc = obj.find("jsonrpc");
	if (it_jsonrpc == obj.end()
	    || !it_jsonrpc->value().is_string()
	    || boost::json::value_to<std::string>(it_jsonrpc->value()) != "2.0") {
		return std::nullopt;
	}

	auto it_method = obj.find("method");
	if (it_method == obj.end() || !it_method->value().is_string()) {
		return std::nullopt;
	}

	JsonRpcRequest req;
	req.method = boost::json::value_to<std::string>(it_method->value());

	auto it_params = obj.find("params");
	if (it_params != obj.end()) {
		req.params = it_params->value();
	} else {
		req.params = boost::json::object{};
	}

	auto it_id = obj.find("id");
	if (it_id != obj.end()) {
		req.id = it_id->value();
	} else {
		req.id = nullptr;
	}

	return req;
}

auto make_response(const JsonRpcResponse &res) -> boost::json::value
{
	boost::json::object j;
	j["jsonrpc"] = "2.0";
	j["id"]      = res.id;

	if (res.error.has_value()) {
		boost::json::object err;
		err["code"]    = res.error->code;
		err["message"] = res.error->message;
		if (res.error->data.has_value()) {
			err["data"] = *res.error->data;
		}
		j["error"] = std::move(err);
	} else if (res.result.has_value()) {
		j["result"] = *res.result;
	}

	return j;
}

auto make_error_response(int code, const std::string &message,
                         boost::json::value id) -> boost::json::value
{
	return make_response(JsonRpcResponse{
		std::move(id),
		std::nullopt,
		JsonRpcError{code, message, std::nullopt},
	});
}

auto make_success_response(boost::json::value id,
                           boost::json::value result) -> boost::json::value
{
	return make_response(JsonRpcResponse{
		std::move(id),
		std::move(result),
		std::nullopt,
	});
}

} // namespace git_mcp::protocol
