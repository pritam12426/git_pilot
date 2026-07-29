#pragma once

#include <git2.h>

#include <memory>
#include <string>
#include <utility>

#include "git/repo.hpp"

namespace git_pilot::git {

// RAII wrapper for git_signature* (author / committer identity).
class GitActor
{
public:
	GitActor() = default;

	GitActor(std::string_view name, std::string_view email)
	{
		git_signature *s = nullptr;
		throw_on_error(git_signature_now(
		    &s, std::string(name).c_str(), std::string(email).c_str()));
		sig_.reset(s);
	}

	GitActor(const GitActor &)            = delete;
	GitActor &operator=(const GitActor &) = delete;

	GitActor(GitActor &&other) noexcept
	    : sig_(std::move(other.sig_))
	{}

	GitActor &operator=(GitActor &&other) noexcept
	{
		if (this != &other) {
			sig_ = std::move(other.sig_);
		}
		return *this;
	}

	[[nodiscard]] git_signature *ptr() const noexcept
	{
		return sig_.get();
	}

	[[nodiscard]] explicit operator bool() const noexcept
	{
		return sig_ != nullptr;
	}

	// Read user.name and user.email from the repo's config.
	static auto from_config(const GitRepo &repo) -> std::optional<GitActor>
	{
		git_config *cfg = nullptr;
		if (git_repository_config(&cfg, repo.ptr()) < 0) {
			return std::nullopt;
		}
		auto cfg_guard = [&] { git_config_free(cfg); };

		auto read = [&](const char *key) -> std::optional<std::string> {
			const char *val = nullptr;
			if (git_config_get_string(&val, cfg, key) < 0 || !val) {
				return std::nullopt;
			}
			return std::string(val);
		};

		auto name  = read("user.name");
		auto email = read("user.email");

		cfg_guard();
		git_config_free(cfg);

		if (!name || !email) {
			return std::nullopt;
		}
		return GitActor(*name, *email);
	}

private:
	struct Deleter {
		void operator()(git_signature *s) const noexcept { git_signature_free(s); }
	};
	std::unique_ptr<git_signature, Deleter> sig_;
};

} // namespace git_pilot::git
