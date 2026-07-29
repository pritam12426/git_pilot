#include "git_mcp/session.hpp"

#include "git_pilot/utils/logger.hpp"

namespace git_mcp {

using namespace git_pilot::utils;

Session::Session(std::unique_ptr<transport::Transport> transport)
    : transport_(std::move(transport))
{}

void Session::run()
{
	LOG_INFO("Session started");

	transport_->run([this](const boost::json::value &msg) {
		on_message(msg);
	});

	LOG_INFO("Session ended");
}

void Session::on_message(const boost::json::value &msg)
{
	auto response = protocol::handle_mcp_message(msg);
	transport_->send(response);
}

} // namespace git_mcp
