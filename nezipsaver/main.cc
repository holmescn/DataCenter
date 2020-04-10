#include <cstdlib>
#include <csignal>
#include <string>
#include <iostream>
#include <cxxopts.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/cfg/env.h>
#include "service.h"

// Resolve hostname to ip
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

constexpr auto PROGRAM_NAME = "NeZipSaver";

Service* g_pService = nullptr;

void signal_handler(int)
{
	if (g_pService) {
		g_pService->Stop();
		g_pService = nullptr;
		spdlog::info("Quit the NeZipSaver...");
	}
	else {
		spdlog::warn("Please wait to quit the NeZipSaver...");
	}
}

std::string get_option_string(cxxopts::ParseResult& args, const char* key1, const char* key2, const std::string& defaultValue)
{
	if (args.count(key1)) {
		return args[key1].as<std::string>();
	}
	else {
		const char* envVal = std::getenv(key2);
		if (envVal) {
			return envVal;
		}
		else {
			return defaultValue;
		}
	}
}

int get_option_int(cxxopts::ParseResult& args, const char* key1, const char* key2, int defaultValue)
{
	if (args.count(key1)) {
		return args[key1].as<int>();
	}
	else {
		const char* envVal = std::getenv(key2);
		if (envVal) {
			return atoi(envVal);
		}
		else {
			return defaultValue;
		}
	}
}

int main(int argc, char** argv)
{
	cxxopts::Options options(PROGRAM_NAME, "Save data from NeZipReceiver to TDengine.");
	options.add_options()
		("h,help", "Show this help.")
		("l,listen", "NNG listen url (e.g. tcp://127.0.0.1:8888).", cxxopts::value<std::string>(), "url")
		("r,receivers", "Number of receivers (default 64).", cxxopts::value<int>(), "N")
		("s,savers", "Number of savers (default 4).", cxxopts::value<int>(), "N")
		("host", "TDengine host (default TDengine).", cxxopts::value<std::string>(), "hostname")
		("ip", "TDengine host ip.", cxxopts::value<std::string>(), "ip")
		("port", "TDengine port (default 6030).", cxxopts::value<int>(), "port")
		("u,user", "TDengine user (default root).", cxxopts::value<std::string>(), "username")
		("p,password", "TDengine password (default empty).", cxxopts::value<std::string>(), "password")
		;

	auto args = options.parse(argc, argv);
	if (args.count("help")) {
		std::cout << options.help() << std::endl;
		exit(0);
	}

	// Create color multi threaded logger
	// and set as the default logger
	auto console_logger = spdlog::stdout_color_mt(PROGRAM_NAME);
	spdlog::set_default_logger(console_logger);

	// change log pattern
	spdlog::set_pattern("[%m-%d %H:%M:%S] [%n] [%^-%L-%$] %v");

	// SPDLOG_LEVEL=info,mylogger=trace
	spdlog::cfg::load_env_levels();

	Service s;
	g_pService = &s;

	signal(SIGINT, signal_handler);

	std::string listen_url = get_option_string(args, "listen", "NEZIP_SAVER_LISTEN", "tcp://0.0.0.0:8001");
	int nReceivers = get_option_int(args, "receivers", "NEZIP_SAVER_RECEIVERS", 64);
	int nSavers = get_option_int(args, "savers", "NEZIP_SAVER_SAVERS", 4);
	
	s.SetNumberOfSavers(nSavers);
	s.SetNumberOfReceivers(nReceivers);
	s.StartReceivers(listen_url.c_str());

	std::string user = get_option_string(args, "user", "NEZIP_SAVER_TD_USER", "root");
	std::string pass = get_option_string(args, "password", "NEZIP_SAVER_TD_PASSWORD", "taosdata");
	int port = get_option_int(args, "port", "NEZIP_SAVER_TD_PORT", 6030);

	std::string ip = get_option_string(args, "ip", "NEZIP_SAVER_TD_IP", "");
	std::string host = get_option_string(args, "host", "NEZIP_SAVER_TD_HOST", "TDengine");
	if (ip == "") {
		struct hostent* h = gethostbyname(host.c_str());
		if (h) {
			struct in_addr* address = reinterpret_cast<struct in_addr*>(h->h_addr);
			ip = inet_ntoa(*address);
		}
		else {
			spdlog::error("Unabled to resolve {}", host);
			exit(1);
		}
	}

	s.StartSavers(ip, user, pass, port);
	
	s.Join();

	return 0;
}
