#include "git_pilot/tools/tool_base.hpp"

#include "git/commands.hpp"

namespace git_pilot::tools {

class LogTool final : public ToolBase
{
public:
	ToolDefinition get_definition() const override
	{
		return {
			"log",
			"Show commit log",
			{
				{"repo_path", "string", "Path to the git repository", true,
				 boost::json::value{{"description", "Repository path"}}},
				{"max_count", "integer", "Maximum number of commits to show", false,
				 boost::json::value{{"default", 10}}},
				{"branch", "string", "Branch or revision to start from", false,
				 boost::json::value{{"default", "HEAD"}}},
			},
		};
	}

	boost::json::value execute(const boost::json::value &args) override
	{
		auto const &obj = args.as_object();
		auto repo_path = boost::json::value_to<std::string>(obj.at("repo_path"));
		auto max_count = obj.contains("max_count") && obj.at("max_count").is_int64()
		                     ? static_cast<int>(obj.at("max_count").as_int64())
		                     : 10;
		std::string branch = obj.contains("branch") && obj.at("branch").is_string()
		                         ? boost::json::value_to<std::string>(obj.at("branch"))
		                         : "HEAD";
		return git::log(repo_path, max_count, branch);
	}
};

std::unique_ptr<ToolBase> create_log_tool()
{
	return std::make_unique<LogTool>();
}

} // namespace git_pilot::tools
