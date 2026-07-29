#include "git_mcp/transport/stdio_transport.hpp"

#include <iostream>

#include "git_pilot/utils/logger.hpp"

namespace git_mcp::transport {

using namespace git_pilot::utils;

void StdioTransport::run(MessageCallback on_message)
{
	running_ = true;

	std::string line;
	while (running_ && std::getline(std::cin, line)) {
		if (line.empty()) {
			continue;
		}

		LOG_TRACE("STDIO >> {}", line);

		try {
			auto msg = boost::json::parse(line);
			on_message(std::move(msg));
		} catch (const std::exception &e) {
			LOG_ERROR("Failed to parse JSON from stdin: {}", e.what());
			boost::json::object err;
			err["jsonrpc"] = "2.0";
			boost::json::object error;
			error["code"]    = -32700;
			error["message"] = "Parse error";
			err["error"]     = std::move(error);
			err["id"]        = nullptr;
			send(err);
		}
	}

	running_ = false;
}

void StdioTransport::send(const boost::json::value &message)
{
	auto text = boost::json::serialize(message);
	LOG_TRACE("STDIO << {}", text);
	std::cout << text << "\n" << std::flush;
}

void StdioTransport::close()
{
	running_ = false;
}

} // namespace git_mcp::transport
