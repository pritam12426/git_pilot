#pragma once

#include <git2.h>

#include <format>
#include <string>
#include <system_error>
#include <utility>

namespace git_pilot::git {

// Thin RAII wrapper for libgit2 error codes.
// Throws std::system_error with the libgit2 error message on failure.
class GitError : public std::system_error
{
public:
	explicit GitError(int rc)
	    : std::system_error(rc, std::generic_category(),
	                        git_error_last() ? git_error_last()->message : "unknown error")
	{}
};

inline void throw_on_error(int rc)
{
	if (rc < 0) [[unlikely]] {
		throw GitError(rc);
	}
}

// RAII wrapper around git_repository*.
// Opens or discovers a git repository from a given path.
class GitRepo
{
public:
	GitRepo() = default;

	explicit GitRepo(std::string_view path)
	{
		git_repository *r = nullptr;
		throw_on_error(git_repository_open_ext(
		    &r, std::string(path).c_str(),
		    GIT_REPOSITORY_OPEN_FROM_ENV, nullptr));
		repo_.reset(r);
	}

	GitRepo(const GitRepo &)            = delete;
	GitRepo &operator=(const GitRepo &) = delete;

	GitRepo(GitRepo &&other) noexcept
	    : repo_(std::move(other.repo_))
	{}

	GitRepo &operator=(GitRepo &&other) noexcept
	{
		if (this != &other) {
			repo_ = std::move(other.repo_);
		}
		return *this;
	}

	[[nodiscard]] git_repository *ptr() const noexcept
	{
		return repo_.get();
	}

	[[nodiscard]] explicit operator bool() const noexcept
	{
		return repo_ != nullptr;
	}

	[[nodiscard]] std::string path() const
	{
		auto *p = git_repository_workdir(repo_.get());
		return p ? std::string(p) : std::string{};
	}

	[[nodiscard]] bool is_bare() const noexcept
	{
		return git_repository_is_bare(repo_.get()) != 0;
	}

	[[nodiscard]] bool is_empty() const noexcept
	{
		return git_repository_is_empty(repo_.get()) != 0;
	}

private:
	struct Deleter {
		void operator()(git_repository *r) const noexcept { git_repository_free(r); }
	};
	std::unique_ptr<git_repository, Deleter> repo_;
};

} // namespace git_pilot::git
