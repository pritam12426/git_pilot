#pragma once

#include <boost/json.hpp>

#include <functional>

namespace git_mcp::transport {

using MessageCallback = std::function<void(boost::json::value)>;

class Transport
{
public:
	virtual ~Transport() = default;

	virtual void run(MessageCallback on_message)                    = 0;
	virtual void send(const boost::json::value &message)            = 0;
	virtual void close()                                            = 0;
};

} // namespace git_mcp::transport
