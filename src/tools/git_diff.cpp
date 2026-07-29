#include "git_pilot/tools/tool_base.hpp"

#include "git/commands.hpp"

namespace git_pilot::tools {

class DiffTool final : public ToolBase
{
public:
	ToolDefinition get_definition() const override
	{
		return {
			"diff",
			"Show changes between commits, working tree, and index",
			{
				{"repo_path", "string", "Path to the git repository", true,
				 boost::json::value{{"description", "Repository path"}}},
				{"target", "string", "Revision to diff against", false,
				 boost::json::value{{"default", "HEAD"}}},
				{"staged", "boolean", "Show staged changes", false,
				 boost::json::value{{"default", false}}},
			},
		};
	}

	boost::json::value execute(const boost::json::value &args) override
	{
		auto const &obj = args.as_object();
		auto repo_path = boost::json::value_to<std::string>(obj.at("repo_path"));
		auto target    = obj.contains("target") && obj.at("target").is_string()
		                     ? boost::json::value_to<std::string>(obj.at("target"))
		                     : "HEAD";
		auto staged    = obj.contains("staged") && obj.at("staged").is_bool()
		                     ? obj.at("staged").as_bool()
		                     : false;
		return git::diff(repo_path, target, staged);
	}
};

std::unique_ptr<ToolBase> create_diff_tool()
{
	return std::make_unique<DiffTool>();
}

} // namespace git_pilot::tools
