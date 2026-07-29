#include <thread>

#include "git_pilot/utils/logger.hpp"

using namespace git_pilot::utils;

int main()
{
	// Initialize logger (stderr by default)
	Logger::instance().init("", LogLevel::Debug);

	// Test all log levels
	LOG_FATAL("Unrecoverable error: {}", "database corrupted");
	LOG_ERROR("Failed to open file: {}", "/etc/config.conf");
	LOG_WARN("Connection timeout, retrying... ({}/3)", 1);
	LOG_INFO("Server started on port {}", 8080);
	LOG_DEBUG("Processing request ID: {}", 12345);
	LOG_TRACE("Parsing token at position {}", 42);

	// Test perror
	FILE *f = fopen("nonexistent.txt", "r");
	if (!f) {
		LOG_PERROR("Failed to open file");
	}

	// Thread-safety test
	std::vector<std::thread> threads;
	for (int i = 0; i < 10; ++i) {
		threads.emplace_back([i]() {
			LOG_INFO("Thread {} says hello!", i);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			LOG_DEBUG("Thread {} says goodbye!", i);
		});
	}

	for (auto &t : threads) {
		t.join();
	}

	Logger::instance().flush();
	return 0;
}
