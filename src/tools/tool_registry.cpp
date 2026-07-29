#include "git_pilot/tools/tool_registry.hpp"

#include <memory>
#include <string>

// Forward-declare tool factory functions
namespace git_pilot::tools {
std::unique_ptr<ToolBase> create_status_tool();
std::unique_ptr<ToolBase> create_log_tool();
std::unique_ptr<ToolBase> create_diff_tool();
std::unique_ptr<ToolBase> create_commit_tool();
} // namespace git_pilot::tools

namespace git_pilot::tools {

ToolRegistry &ToolRegistry::instance() noexcept
{
	static ToolRegistry reg;
	return reg;
}

ToolRegistry::ToolRegistry()
{
	register_tool("status", create_status_tool());
	register_tool("log",    create_log_tool());
	register_tool("diff",   create_diff_tool());
	register_tool("commit", create_commit_tool());
}

void ToolRegistry::register_tool(std::string name, std::unique_ptr<ToolBase> tool)
{
	tools_[std::move(name)] = std::move(tool);
}

ToolBase *ToolRegistry::find(std::string_view name) const
{
	auto it = tools_.find(std::string(name));
	return it != tools_.end() ? it->second.get() : nullptr;
}

auto ToolRegistry::all_definitions() const -> std::vector<ToolDefinition>
{
	std::vector<ToolDefinition> defs;
	defs.reserve(tools_.size());
	for (auto &[_, tool] : tools_) {
		defs.push_back(tool->get_definition());
	}
	return defs;
}

} // namespace git_pilot::tools
