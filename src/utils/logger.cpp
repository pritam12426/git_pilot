/**
 * @file log.cpp
 * @brief Modern C++20 thread-safe logger implementation
 */

#include "git_pilot/utils/logger.hpp"

#include <iostream>
#include <mutex>

#ifdef _WIN32
#include <io.h>  // _isatty, _fileno
#else
#include <unistd.h>  // isatty, fileno
#endif


namespace git_pilot::utils {

// ─── Logger Implementation ────────────────────────────────────────────────────

Logger &Logger::instance() noexcept
{
	static Logger instance;
	return instance;
}

Logger::Logger()
    : output_stream_(&std::cerr)
    , use_color_(is_terminal(std::cerr))
{}

Logger::~Logger()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (file_stream_.is_open()) {
		file_stream_.close();
	}
}

void Logger::init(std::string_view log_file, LogLevel level)
{
	std::lock_guard<std::mutex> lock(mutex_);

	// Close existing file
	if (file_stream_.is_open()) {
		file_stream_.close();
	}

	if (log_file.empty()) {
		// Use stderr
		use_file_      = false;
		output_stream_ = &std::cerr;
		use_color_     = is_terminal(std::cerr);
	} else {
		// Open log file
		file_stream_.open(log_file.data(), std::ios::out | std::ios::app);
		if (!file_stream_.is_open()) {
			// Fallback to stderr
			use_file_      = false;
			output_stream_ = &std::cerr;
			use_color_     = is_terminal(std::cerr);

			// Warn about failure (using raw cerr to avoid recursion)
			std::cerr << "\033[33m[LOG] Warning: Could not open log file '" << log_file
			          << "', falling back to stderr\033[0m\n";
		} else {
			use_file_      = true;
			output_stream_ = &file_stream_;
			use_color_     = false;  // Files don't use colors
		}
	}

	min_level_.store(level, std::memory_order_release);
	initialized_ = true;
}

void Logger::set_level(LogLevel level) noexcept
{
	min_level_.store(level, std::memory_order_release);
}

LogLevel Logger::get_level() const noexcept
{
	return min_level_.load(std::memory_order_acquire);
}

bool Logger::is_level_enabled(LogLevel level) const noexcept
{
	return level <= min_level_.load(std::memory_order_acquire);
}

void Logger::log_impl(LogLevel                    level,
                      std::string_view            fmt,
                      const std::source_location &loc,
#ifdef LOG_USE_FMT
                      fmt::format_args args)
#else
                      std::format_args args)
#endif
{
	std::string message;
	try {
#ifdef LOG_USE_FMT
		message = fmt::vformat(fmt, args);
#else
		message = std::vformat(fmt, args);
#endif
	} catch (const std::exception &e) {
		message = std::string("[FORMAT ERROR] ") + e.what();
	}
	write_log_entry(level, message, loc);
}

void Logger::log_perror_impl(std::string_view            fmt,
                             const std::source_location &loc,
#ifdef LOG_USE_FMT
                             fmt::format_args args)
#else
                             std::format_args args)
#endif
{
	auto        err = errno;
	std::string message;
	try {
#ifdef LOG_USE_FMT
		message = fmt::vformat(fmt, args);
#else
		message = std::vformat(fmt, args);
#endif
	} catch (const std::exception &e) {
		message = std::string("[FORMAT ERROR] ") + e.what();
	}
	message += ": ";
	message += std::strerror(err);
	write_log_entry(LogLevel::Error, message, loc);
}

bool Logger::use_color() const noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	return use_color_;
}

void Logger::flush()
{
	std::lock_guard<std::mutex> lock(mutex_);
	output_stream_->flush();
}

bool Logger::is_terminal(std::ostream &stream)
{
	if (&stream == &std::cout) {
#ifdef _WIN32
		return ::_isatty(::_fileno(stdout)) != 0;
#else
		return ::isatty(fileno(stdout)) != 0;
#endif
	} else if (&stream == &std::cerr || &stream == &std::clog) {
#ifdef _WIN32
		return ::_isatty(::_fileno(stderr)) != 0;
#else
		return ::isatty(fileno(stderr)) != 0;
#endif
	}
	return false;
}

std::string Logger::get_timestamp()
{
	using namespace std::chrono;

	auto now  = system_clock::now();
	auto time = system_clock::to_time_t(now);
	auto us   = duration_cast<microseconds>(now.time_since_epoch()) % 1000000;

	std::tm tm_buf;
#ifdef _WIN32
	localtime_s(&tm_buf, &time);
#else
	localtime_r(&time, &tm_buf);
#endif

	std::string result(32, '\0');
	auto        len = std::strftime(result.data(), result.size(), "%H:%M:%S", &tm_buf);
	result.resize(len);

	// Use string concatenation to avoid std::format dependency
	std::string timestamp  = "[";
	timestamp             += result;
	timestamp             += ".";

	// Format microseconds with 6 digits
	char us_buf[8];
	snprintf(us_buf, sizeof(us_buf), "%06d", static_cast<int>(us.count()));
	timestamp += us_buf;
	timestamp += "] ";

	return timestamp;
}

bool Logger::is_source_location_enabled() noexcept
{
#ifdef LOG_SHOW_SOURCE_LOCATION
	return true;
#else
	return false;
#endif
}

void Logger::write_log_entry(LogLevel                    level,
                             std::string_view            message,
                             const std::source_location &loc)
{
	std::lock_guard<std::mutex> lock(mutex_);

#ifdef LOG_SHOW_TIME_STAMP
	if (use_color_) {
		*output_stream_ << "\033[2m";  // Dim
	}
	*output_stream_ << get_timestamp();
	if (use_color_) {
		*output_stream_ << "\033[0m";  // Reset
	}
#endif

	// Log level label with color
	if (use_color_) {
		*output_stream_ << '[';
		*output_stream_ << log_level_to_color(level);
		*output_stream_ << log_level_to_string(level);
		*output_stream_ << "\033[0m";
		*output_stream_ << "] ";
	} else {
		*output_stream_ << '[' << log_level_to_string(level) << "] ";
	}

#ifdef LOG_SHOW_SOURCE_LOCATION
	if (use_color_) {
		*output_stream_ << "\033[2m";  // Dim
	}
	{
		*output_stream_ << '[' << loc.file_name() << ':' << loc.line() << ':' << loc.function_name()
		                << ']';
	}
	if (use_color_) {
		*output_stream_ << "\033[0m ";
	} else {
		*output_stream_ << ' ';
	}
#endif

	*output_stream_ << message << '\n';
}

}  // namespace git_pilot::utils
