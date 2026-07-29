#include "git/commands.hpp"

#include <ctime>
#include <format>
#include <vector>

#include "git/actor.hpp"
#include "git/repo.hpp"

namespace git_pilot::git {

// ─── helpers ──────────────────────────────────────────────────────────────────

namespace {

[[nodiscard]] auto oid_to_hex(git_oid const &id) -> std::string
{
	static constexpr char hex_chars[] = "0123456789abcdef";
	std::string hex(40, '\0');
	for (int i = 0; i < 20; ++i) {
		hex[static_cast<std::size_t>(i * 2)]     = hex_chars[(id.id[i] >> 4) & 0xf];
		hex[static_cast<std::size_t>(i * 2 + 1)] = hex_chars[id.id[i] & 0xf];
	}
	return hex;
}

// RAII deleters for libgit2 types
struct ObjectDeleter {
	void operator()(git_object *o) const noexcept { git_object_free(o); }
};
struct DiffDeleter {
	void operator()(git_diff *d) const noexcept { git_diff_free(d); }
};
struct WalkDeleter {
	void operator()(git_revwalk *w) const noexcept { git_revwalk_free(w); }
};
struct TreeDeleter {
	void operator()(git_tree *t) const noexcept { git_tree_free(t); }
};
struct CommitDeleter {
	void operator()(git_commit *c) const noexcept { git_commit_free(c); }
};

using ObjPtr   = std::unique_ptr<git_object, ObjectDeleter>;
using DiffPtr  = std::unique_ptr<git_diff, DiffDeleter>;
using WalkPtr  = std::unique_ptr<git_revwalk, WalkDeleter>;
using TreePtr  = std::unique_ptr<git_tree, TreeDeleter>;
using CommitPtr = std::unique_ptr<git_commit, CommitDeleter>;

// Resolve a revision string (HEAD, branch name, SHA) to a git_oid.
[[nodiscard]] auto resolve_rev(git_repository *repo, std::string_view rev) -> git_oid
{
	git_object *obj = nullptr;
	throw_on_error(git_revparse_single(&obj, repo, std::string(rev).c_str()));
	ObjPtr obj_guard(obj);
	return *git_object_id(obj);
}

} // anonymous namespace

// ─── status ───────────────────────────────────────────────────────────────────

boost::json::value status(std::string_view repo_path)
{
	GitRepo repo(repo_path);

	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED
	           | GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX
	           | GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

	boost::json::array entries;

	auto callback = [](const char *path, unsigned int flags, void *payload) {
		auto &out = *static_cast<boost::json::array *>(payload);
		boost::json::object entry;
		entry["path"] = path;

		if (flags & GIT_STATUS_INDEX_NEW)            entry["index_status"] = "added";
		else if (flags & GIT_STATUS_INDEX_MODIFIED)  entry["index_status"] = "modified";
		else if (flags & GIT_STATUS_INDEX_DELETED)   entry["index_status"] = "deleted";
		else if (flags & GIT_STATUS_INDEX_RENAMED)   entry["index_status"] = "renamed";
		else if (flags & GIT_STATUS_INDEX_TYPECHANGE) entry["index_status"] = "typechange";

		if (flags & GIT_STATUS_WT_NEW)               entry["worktree_status"] = "added";
		else if (flags & GIT_STATUS_WT_MODIFIED)     entry["worktree_status"] = "modified";
		else if (flags & GIT_STATUS_WT_DELETED)      entry["worktree_status"] = "deleted";
		else if (flags & GIT_STATUS_WT_RENAMED)      entry["worktree_status"] = "renamed";
		else if (flags & GIT_STATUS_WT_TYPECHANGE)   entry["worktree_status"] = "typechange";

		if (flags & GIT_STATUS_IGNORED)    entry["ignored"] = true;
		if (flags & GIT_STATUS_CONFLICTED) entry["conflicted"] = true;

		out.push_back(std::move(entry));
		return 0;
	};

	throw_on_error(git_status_foreach_ext(
	    repo.ptr(), &opts, callback, &entries));

	boost::json::object result;
	result["files"] = std::move(entries);
	return result;
}

// ─── log ──────────────────────────────────────────────────────────────────────

boost::json::value log(std::string_view repo_path, int max_count, std::string_view branch)
{
	GitRepo repo(repo_path);

	git_revwalk *w = nullptr;
	throw_on_error(git_revwalk_new(&w, repo.ptr()));
	WalkPtr walk(w);

	git_revwalk_sorting(walk.get(), GIT_SORT_TIME | GIT_SORT_TOPOLOGICAL);

	auto start_oid = resolve_rev(repo.ptr(), branch);
	throw_on_error(git_revwalk_push(walk.get(), &start_oid));

	boost::json::array commits;
	int count = 0;
	git_oid oid;

	while (git_revwalk_next(&oid, walk.get()) == 0 && count < max_count) {
		git_commit *c = nullptr;
		if (git_commit_lookup(&c, repo.ptr(), &oid) < 0) {
			break;
		}
		CommitPtr commit(c);

		auto *author    = git_commit_author(commit.get());
		auto *committer = git_commit_committer(commit.get());

		boost::json::object entry;
		entry["hash"]            = oid_to_hex(oid);
		entry["author_name"]     = author ? author->name : "";
		entry["author_email"]    = author ? author->email : "";
		entry["committer_name"]  = committer ? committer->name : "";
		entry["committer_email"] = committer ? committer->email : "";
		entry["timestamp"]       = static_cast<std::int64_t>(git_commit_time(commit.get()));
		entry["message"]         = git_commit_message(commit.get())
		                           ? git_commit_message(commit.get()) : "";

		commits.push_back(std::move(entry));
		++count;
	}

	boost::json::object result;
	result["commits"] = std::move(commits);
	return result;
}

// ─── diff ─────────────────────────────────────────────────────────────────────

boost::json::value diff(std::string_view repo_path, std::string_view target, bool staged)
{
	GitRepo repo(repo_path);

	git_diff *raw_diff = nullptr;

	if (staged) {
		git_object *obj = nullptr;
		throw_on_error(git_revparse_single(&obj, repo.ptr(), std::string(target).c_str()));
		ObjPtr obj_guard(obj);

		if (git_object_type(obj) != GIT_OBJECT_COMMIT) {
			throw GitError(git_error_code(GIT_ENOTFOUND));
		}
		auto *commit = reinterpret_cast<git_commit *>(obj);
		git_tree *head_tree = nullptr;
		throw_on_error(git_commit_tree(&head_tree, commit));
		TreePtr head_tree_guard(head_tree);

		throw_on_error(git_diff_tree_to_index(
		    &raw_diff, repo.ptr(), head_tree, nullptr, nullptr));
	} else {
		git_object *obj = nullptr;
		throw_on_error(git_revparse_single(&obj, repo.ptr(), std::string(target).c_str()));
		ObjPtr obj_guard(obj);

		if (git_object_type(obj) != GIT_OBJECT_COMMIT) {
			throw GitError(git_error_code(GIT_ENOTFOUND));
		}
		auto *commit = reinterpret_cast<git_commit *>(obj);
		git_tree *head_tree = nullptr;
		throw_on_error(git_commit_tree(&head_tree, commit));
		TreePtr head_tree_guard(head_tree);

		throw_on_error(git_diff_tree_to_workdir(
		    &raw_diff, repo.ptr(), head_tree, nullptr));
	}

	DiffPtr diff_guard(raw_diff);

	git_buf buf = GIT_BUF_INIT_CONST(0, nullptr);
	throw_on_error(git_diff_to_buf(&buf, raw_diff, GIT_DIFF_FORMAT_PATCH));
	std::string text(buf.ptr, buf.size);
	git_buf_dispose(&buf);

	boost::json::object result;
	result["patch"] = std::move(text);
	return result;
}

// ─── commit ───────────────────────────────────────────────────────────────────

boost::json::value commit(std::string_view repo_path,
                           std::string_view message,
                           std::string_view author_name,
                           std::string_view author_email)
{
	GitRepo repo(repo_path);

	GitActor actor(author_name, author_email);

	git_index *idx = nullptr;
	throw_on_error(git_repository_index(&idx, repo.ptr()));
	auto idx_guard = [&] { git_index_free(idx); };

	git_oid tree_id;
	throw_on_error(git_index_write_tree(&tree_id, idx));

	git_tree *tree = nullptr;
	throw_on_error(git_tree_lookup(&tree, repo.ptr(), &tree_id));
	TreePtr tree_guard(tree);

	git_oid parent_id;
	std::array<git_commit *, 1> parents{};
	int parent_count = 0;

	if (git_reference_name_to_id(&parent_id, repo.ptr(), "HEAD") == 0) {
		git_commit *parent = nullptr;
		if (git_commit_lookup(&parent, repo.ptr(), &parent_id) == 0) {
			parents[0] = parent;
			parent_count = 1;
		}
	}

	git_oid commit_id;
	throw_on_error(git_commit_create_v(
	    &commit_id, repo.ptr(), "HEAD",
	    actor.ptr(), actor.ptr(),
	    nullptr, std::string(message).c_str(),
	    tree_guard.get(), static_cast<size_t>(parent_count),
	    parents[0]));

	if (parents[0]) {
		git_commit_free(parents[0]);
	}

	idx_guard();
	git_index_free(idx);

	boost::json::object result;
	result["hash"]    = oid_to_hex(commit_id);
	result["message"] = std::string(message);
	return result;
}

} // namespace git_pilot::git
