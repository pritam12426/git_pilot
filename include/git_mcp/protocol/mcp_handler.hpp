#pragma once

#include <boost/json.hpp>

namespace git_mcp::protocol {

[[nodiscard]] auto handle_mcp_message(const boost::json::value &request) -> boost::json::value;

} // namespace git_mcp::protocol
