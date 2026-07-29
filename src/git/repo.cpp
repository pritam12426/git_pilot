#include "git/repo.hpp"

namespace git_pilot::git {

static auto init_count = (git_libgit2_init(), 0);
[[maybe_unused]] static auto shutdown_count = init_count;

} // namespace git_pilot::git
