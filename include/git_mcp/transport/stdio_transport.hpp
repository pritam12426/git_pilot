#pragma once

#include "git_mcp/transport/transport.hpp"

namespace git_mcp::transport {

class StdioTransport final : public Transport
{
public:
	StdioTransport() = default;

	void run(MessageCallback on_message) override;
	void send(const boost::json::value &message) override;
	void close() override;

private:
	bool running_{false};
};

} // namespace git_mcp::transport
