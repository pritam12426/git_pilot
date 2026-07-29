#pragma once

#include "git_mcp/protocol/mcp_handler.hpp"
#include "git_mcp/transport/transport.hpp"

#include <memory>

namespace git_mcp {

class Session
{
public:
	explicit Session(std::unique_ptr<transport::Transport> transport);

	void run();

private:
	void on_message(const boost::json::value &msg);

	std::unique_ptr<transport::Transport> transport_;
};

} // namespace git_mcp
