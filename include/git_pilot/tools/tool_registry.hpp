#pragma once

#include "git_pilot/tools/tool_base.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace git_pilot::tools {

class ToolRegistry
{
public:
	static ToolRegistry &instance() noexcept;

	void register_tool(std::string name, std::unique_ptr<ToolBase> tool);

	[[nodiscard]] ToolBase *find(std::string_view name) const;

	[[nodiscard]] auto all_definitions() const -> std::vector<ToolDefinition>;

private:
	ToolRegistry();

	std::unordered_map<std::string, std::unique_ptr<ToolBase>> tools_;
};

} // namespace git_pilot::tools
