#pragma once

#include <boost/json.hpp>

#include <string_view>

namespace git_pilot::git {

[[nodiscard]] boost::json::value status(std::string_view repo_path);

[[nodiscard]] boost::json::value log(std::string_view repo_path,
                                     int max_count = 10,
                                     std::string_view branch = "HEAD");

[[nodiscard]] boost::json::value diff(std::string_view repo_path,
                                      std::string_view target = "HEAD",
                                      bool staged = false);

[[nodiscard]] boost::json::value commit(std::string_view repo_path,
                                        std::string_view message,
                                        std::string_view author_name,
                                        std::string_view author_email);

} // namespace git_pilot::git
