#pragma once

/**
 * @file log.hpp
 * @brief Modern C++20 thread-safe logger interface
 *
 * Usage:
 *   #include "log.hpp"
 *   LOG_INFO("Server started on port {}", 8080);
 */

#include <fstream>
#include <source_location>

#ifdef LOG_USE_FMT
#  include <fmt/core.h>
#else
#  include <format>
#endif


namespace git_pilot::utils {

// ─── Enums ────────────────────────────────────────────────────────────────────

enum class LogLevel : uint8_t {
	Off   = 0,
	Fatal = 1,
	Error = 2,
	Warn  = 3,
	Info  = 4,
	Debug = 5,
	Trace = 6
};

// Convert LogLevel to string_view
[[nodiscard]] inline constexpr std::string_view log_level_to_string(LogLevel level) noexcept
{
	using enum LogLevel;
	switch (level) {
		case Off:   return "OFF  ";
		case Fatal: return "FATAL";
		case Error: return "ERROR";
		case Warn:  return "WARN ";
		case Info:  return "INFO ";
		case Debug: return "DEBUG";
		case Trace: return "TRACE";
		default:    return "UNKWN";
	}
}

// Convert LogLevel to ANSI color code
[[nodiscard]] inline constexpr std::string_view log_level_to_color(LogLevel level) noexcept
{
	using enum LogLevel;
	switch (level) {
		case Fatal: return "\033[1;91m";  // Bright Bold Red
		case Error: return "\033[1;31m";  // Bold Red
		case Warn:  return "\033[1;33m";  // Bold Yellow
		case Info:  return "\033[1;32m";  // Bold Green
		case Debug: return "\033[1;36m";  // Bold Cyan
		case Trace: return "\033[1;35m";  // Bold Magenta
		default:    return "\033[1;37m";  // Bold White
	}
}

// Stream operator for LogLevel (needed by Boost.Program_options default_value rendering)
inline std::ostream &operator<<(std::ostream &os, LogLevel level)
{
	auto s = log_level_to_string(level);
	while (!s.empty() && s.back() == ' ') s.remove_suffix(1);
	return os << s;
}

// ─── Logger Class ───────────────────────────────────────────────────────────

class Logger
{
public:
	// Singleton access
	[[nodiscard]] static Logger &instance() noexcept;

	// Delete copy/move
	Logger(const Logger &)            = delete;
	Logger(Logger &&)                 = delete;
	Logger &operator=(const Logger &) = delete;
	Logger &operator=(Logger &&)      = delete;

	// ─── Public API ──────────────────────────────────────────────────────

	void init(std::string_view log_file = "", LogLevel level = LogLevel::Info);
	void set_level(LogLevel level) noexcept;
	[[nodiscard]] LogLevel get_level() const noexcept;
	[[nodiscard]] bool is_level_enabled(LogLevel level) const noexcept;
	[[nodiscard]] bool use_color() const noexcept;
	void flush();

	// Core logging function (variadic template forwarding — defined inline)
	template<typename... Args>
	void log(LogLevel         level,
	         std::string_view fmt,
	         const std::source_location &loc,
	         Args &&...args)
	{
		if (level > min_level_.load(std::memory_order_acquire) || level == LogLevel::Off) {
			return;
		}
#ifdef LOG_USE_FMT
		log_impl(level, fmt, loc, fmt::make_format_args(args...));
#else
		log_impl(level, fmt, loc, std::make_format_args(args...));
#endif
	}

	// Perror-style logging (variadic template forwarding)
	template<typename... Args>
	void log_perror(std::string_view fmt,
	                const std::source_location &loc,
	                Args &&...args)
	{
#ifdef LOG_USE_FMT
		log_perror_impl(fmt, loc, fmt::make_format_args(args...));
#else
		log_perror_impl(fmt, loc, std::make_format_args(args...));
#endif
	}

private:
	// Private constructor/destructor
	Logger();
	~Logger();

	// ─── Internal Helpers ──────────────────────────────────────────────────

	static bool is_terminal(std::ostream &stream);
	static std::string get_timestamp();
	static bool is_source_location_enabled() noexcept;

	void write_log_entry(LogLevel level, std::string_view message, const std::source_location &loc);

#ifdef LOG_USE_FMT
	void log_impl(LogLevel, std::string_view, const std::source_location&, fmt::format_args);
	void log_perror_impl(std::string_view, const std::source_location&, fmt::format_args);
#else
	void log_impl(LogLevel, std::string_view, const std::source_location&, std::format_args);
	void log_perror_impl(std::string_view, const std::source_location&, std::format_args);
#endif

	// ─── Member Variables ──────────────────────────────────────────────────

	mutable std::mutex    mutex_;
	std::atomic<LogLevel> min_level_{ LogLevel::Info };
	std::ostream         *output_stream_{ nullptr };
	std::ofstream         file_stream_;
	bool                  use_file_{ false };
	bool                  use_color_{ false };
	bool                  initialized_{ false };
};

} // namespace git_pilot::utils


// ─── Public Macros ────────────────────────────────────────────────────────────
#define LOG_LEVEL_IS_ENABLED(level) \
	::git_pilot::utils::Logger::instance().is_level_enabled(level)

#define LOG_AT(level, fmt, ...)                                                     \
	do {                                                                            \
		if (::git_pilot::utils::Logger::instance().is_level_enabled(level))         \
			::git_pilot::utils::Logger::instance().log(                             \
				level, fmt, std::source_location::current()                         \
				__VA_OPT__(,) __VA_ARGS__);                                         \
	} while (0)

#define LOG_FATAL(fmt, ...)                                                         \
	do {                                                                            \
		if (::git_pilot::utils::Logger::instance().is_level_enabled(                \
			    ::git_pilot::utils::LogLevel::Fatal))                               \
			::git_pilot::utils::Logger::instance().log(                             \
				::git_pilot::utils::LogLevel::Fatal, fmt,                           \
				std::source_location::current()                                     \
				__VA_OPT__(,) __VA_ARGS__);                                         \
	} while (0)

#define LOG_ERROR(fmt, ...)                                                         \
	do {                                                                            \
		if (::git_pilot::utils::Logger::instance().is_level_enabled(                \
			    ::git_pilot::utils::LogLevel::Error))                               \
			::git_pilot::utils::Logger::instance().log(                             \
				::git_pilot::utils::LogLevel::Error, fmt,                           \
				std::source_location::current()                                     \
				__VA_OPT__(,) __VA_ARGS__);                                         \
	} while (0)

#define LOG_WARN(fmt, ...)                                                          \
	do {                                                                            \
		if (::git_pilot::utils::Logger::instance().is_level_enabled(                \
			    ::git_pilot::utils::LogLevel::Warn))                                \
			::git_pilot::utils::Logger::instance().log(                             \
				::git_pilot::utils::LogLevel::Warn, fmt,                            \
				std::source_location::current()                                     \
				__VA_OPT__(,) __VA_ARGS__);                                         \
	} while (0)

#define LOG_INFO(fmt, ...)                                                          \
	do {                                                                            \
		if (::git_pilot::utils::Logger::instance().is_level_enabled(                \
			    ::git_pilot::utils::LogLevel::Info))                                \
			::git_pilot::utils::Logger::instance().log(                             \
				::git_pilot::utils::LogLevel::Info, fmt,                            \
				std::source_location::current()                                     \
				__VA_OPT__(,) __VA_ARGS__);                                         \
	} while (0)

#define LOG_DEBUG(fmt, ...)                                                         \
	do {                                                                            \
		if (::git_pilot::utils::Logger::instance().is_level_enabled(                \
			    ::git_pilot::utils::LogLevel::Debug))                               \
			::git_pilot::utils::Logger::instance().log(                             \
				::git_pilot::utils::LogLevel::Debug, fmt,                           \
				std::source_location::current()                                     \
				__VA_OPT__(,) __VA_ARGS__);                                         \
	} while (0)

#define LOG_TRACE(fmt, ...)                                                         \
	do {                                                                            \
		if (::git_pilot::utils::Logger::instance().is_level_enabled(                \
			    ::git_pilot::utils::LogLevel::Trace))                               \
			::git_pilot::utils::Logger::instance().log(                             \
				::git_pilot::utils::LogLevel::Trace, fmt,                           \
				std::source_location::current()                                     \
				__VA_OPT__(,) __VA_ARGS__);                                         \
	} while (0)

#define LOG_PERROR(fmt, ...)                                                        \
	do {                                                                            \
		if (::git_pilot::utils::Logger::instance().is_level_enabled(                \
			    ::git_pilot::utils::LogLevel::Error))                               \
			::git_pilot::utils::Logger::instance().log_perror(                      \
				fmt, std::source_location::current()                                \
				__VA_OPT__(,) __VA_ARGS__);                                         \
	} while (0)
