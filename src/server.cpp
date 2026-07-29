#include "git_mcp/server.hpp"

#include <boost/asio.hpp>

#include "git_pilot/utils/logger.hpp"

namespace asio = boost::asio;
using namespace git_pilot::utils;

namespace git_mcp {

Server::Server(std::string host, std::string port)
    : host_(std::move(host))
    , port_(std::move(port))
{}

void Server::run()
{
	asio::io_context io_ctx;

	auto endpoint = asio::ip::tcp::endpoint(
		asio::ip::make_address(host_),
		static_cast<unsigned short>(std::stoul(port_)));

	asio::ip::tcp::acceptor acceptor(io_ctx, endpoint);

	LOG_INFO("MCP server listening on {}:{}", host_, port_);

	for (;;) {
		asio::ip::tcp::socket socket(io_ctx);
		acceptor.accept(socket);

		LOG_INFO("Accepted connection from {}",
		         socket.remote_endpoint().address().to_string());

		socket.close();
	}
}

} // namespace git_mcp
