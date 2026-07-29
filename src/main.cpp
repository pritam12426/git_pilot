#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <boost/any.hpp>
#include <boost/program_options.hpp>

#include "git_pilot/project_config.hpp"
#include "git_pilot/utils/logger.hpp"

#include "git_mcp/session.hpp"
#include "git_mcp/transport/stdio_transport.hpp"

#ifdef ENABLE_HTTP
#include "git_mcp/server.hpp"
#endif

namespace po = boost::program_options;
using namespace git_pilot::utils;

// Custom validator so --log-level maps directly to LogLevel
namespace git_pilot::utils {

void validate(boost::any &v,
              const std::vector<std::string> &values,
              LogLevel * /*target_type*/, int /*unused*/)
{
	po::validators::check_first_occurrence(v);
	auto const &s = po::validators::get_single_string(values);

	if (s == "off")        v = LogLevel::Off;
	else if (s == "fatal") v = LogLevel::Fatal;
	else if (s == "error") v = LogLevel::Error;
	else if (s == "warn")  v = LogLevel::Warn;
	else if (s == "info")  v = LogLevel::Info;
	else if (s == "debug") v = LogLevel::Debug;
	else if (s == "trace") v = LogLevel::Trace;
	else throw po::validation_error(po::validation_error::invalid_option_value, "log-level");
}

} // namespace git_pilot::utils

auto main(int argc, char *argv[]) -> int
{
	po::options_description desc("Usage: git_pilot_server [mode] [options]", 120);
	desc.add_options()
		("help,?",     "Show this help message")
		("version,V",  "Show version information")
		("mode",       po::value<std::string>()->default_value("stdio"),
		               "Server mode (stdio, http)")
		("log-level,L",  po::value<LogLevel>()->default_value(LogLevel::Info),
		               "Log level (off, fatal, error, warn, info, debug, trace)")
		("log-file,F",  po::value<std::string>(),
		               "Write logs to file instead of stderr")
#ifdef ENABLE_HTTP
		("host,H",  po::value<std::string>(),
		            "Host address to bind the HTTP MCP server")
		("port,P",  po::value<std::string>(),
		            "Port to bind the HTTP MCP server")
#endif
	;

	po::positional_options_description pos;
	pos.add("mode", 1);

	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv)
			.options(desc).positional(pos).run(), vm);
		po::notify(vm);
	} catch (const po::unknown_option &e) {
		std::cerr << "Error: " << e.what() << "\n\n" << desc << '\n';
		return EXIT_FAILURE;
	} catch (const po::ambiguous_option &e) {
		std::cerr << "Error: " << e.what() << "\n\n" << desc << '\n';
		return EXIT_FAILURE;
	} catch (const po::validation_error &e) {
		std::cerr << "Error: " << e.what() << "\n\n" << desc << '\n';
		return EXIT_FAILURE;
	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << "\n\n" << desc << '\n';
		return EXIT_FAILURE;
	}

	if (vm.count("help")) {
		std::cout << desc << '\n';
		return EXIT_SUCCESS;
	}

	if (vm.count("version")) {
		std::cout << "GitPilot " << GITPILOT_VERSION << '\n';
		return EXIT_SUCCESS;
	}

	auto log_level = vm["log-level"].as<LogLevel>();
	auto log_file  = vm.count("log-file") ? vm["log-file"].as<std::string>() : std::string{};

	Logger::instance().init(log_file, log_level);

	auto mode = vm["mode"].as<std::string>();

	if (mode == "http") {
#ifndef ENABLE_HTTP
		std::cerr << "Error: HTTP mode not available (build with -DENABLE_HTTP=ON)\n";
		return EXIT_FAILURE;
#else
		auto host = vm.count("host") ? vm["host"].as<std::string>() : std::string{};
		auto port = vm.count("port") ? vm["port"].as<std::string>() : std::string{};
		if (host.empty()) host = "0.0.0.0";
		if (port.empty()) port = "8080";

		LOG_INFO("GitPilot {} starting in HTTP mode on {}:{}", GITPILOT_VERSION, host, port);

		git_mcp::Server server(host, port);
		server.run();
		return EXIT_SUCCESS;
#endif
	}

	if (mode != "stdio") {
		std::cerr << "Error: unknown mode '" << mode << "' (expected stdio or http)\n";
		return EXIT_FAILURE;
	}

	LOG_INFO("GitPilot {} starting in stdio mode", GITPILOT_VERSION);

	auto transport = std::make_unique<git_mcp::transport::StdioTransport>();
	git_mcp::Session session(std::move(transport));
	session.run();

	return EXIT_SUCCESS;
}
