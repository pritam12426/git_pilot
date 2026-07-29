#include "git_pilot/tools/tool_base.hpp"

#include "git/commands.hpp"

namespace git_pilot::tools {

class StatusTool final : public ToolBase
{
public:
	ToolDefinition get_definition() const override
	{
		return {
			"status",
			"Show the working tree status",
			{
				{"repo_path", "string", "Path to the git repository", true,
				 boost::json::value{{"description", "Repository path"}}},
			},
		};
	}

	boost::json::value execute(const boost::json::value &args) override
	{
		auto const &obj = args.as_object();
		return git::status(
		    boost::json::value_to<std::string>(obj.at("repo_path")));
	}
};

std::unique_ptr<ToolBase> create_status_tool()
{
	return std::make_unique<StatusTool>();
}

} // namespace git_pilot::tools
