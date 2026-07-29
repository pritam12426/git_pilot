#pragma once

#include <string>

namespace git_mcp {

class Server
{
public:
	Server(std::string host, std::string port);

	Server(const Server &)            = delete;
	Server(Server &&)                 = delete;
	Server &operator=(const Server &) = delete;
	Server &operator=(Server &&)      = delete;

	void run();

private:
	std::string host_;
	std::string port_;
};

} // namespace git_mcp
