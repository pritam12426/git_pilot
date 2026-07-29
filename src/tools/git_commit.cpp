#include "git_pilot/tools/tool_base.hpp"

#include "git/commands.hpp"

namespace git_pilot::tools {

class CommitTool final : public ToolBase
{
public:
	ToolDefinition get_definition() const override
	{
		return {
			"commit",
			"Create a new commit",
			{
				{"repo_path",   "string", "Path to the git repository", true,
				 boost::json::value{{"description", "Repository path"}}},
				{"message",     "string", "Commit message", true,
				 boost::json::value{{"description", "Commit message"}}},
				{"author_name", "string", "Author name", false,
				 boost::json::value{{"description", "Author name"}}},
				{"author_email","string", "Author email", false,
				 boost::json::value{{"description", "Author email"}}},
			},
		};
	}

	boost::json::value execute(const boost::json::value &args) override
	{
		auto const &obj = args.as_object();
		auto repo_path   = boost::json::value_to<std::string>(obj.at("repo_path"));
		auto message     = boost::json::value_to<std::string>(obj.at("message"));
		auto author_name = obj.contains("author_name") && obj.at("author_name").is_string()
		                       ? boost::json::value_to<std::string>(obj.at("author_name"))
		                       : std::string{};
		auto author_email = obj.contains("author_email") && obj.at("author_email").is_string()
		                        ? boost::json::value_to<std::string>(obj.at("author_email"))
		                        : std::string{};
		return git::commit(repo_path, message, author_name, author_email);
	}
};

std::unique_ptr<ToolBase> create_commit_tool()
{
	return std::make_unique<CommitTool>();
}

} // namespace git_pilot::tools
