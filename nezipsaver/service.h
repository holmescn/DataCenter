#pragma once
#include <chrono>
#include <future>
#include <forward_list>
#include <cxxopts.hpp>
#include <concurrentqueue.h>
#include <nng/nng.h>
#include <spdlog/spdlog.h>
#include "receiver.h"
#include "saver.h"


class Service
{
	BufferQueue_t m_bufferQueue;
	nng_socket m_sock;
	std::forward_list<Receiver> m_receivers;
	std::forward_list<Saver> m_savers;
	size_t m_nReceivers;
	size_t m_nSavers;

	std::promise<void> m_pExit;
	std::shared_future<void> m_fExit;
public:
	void StartReceivers(const char *url);
	void StartSavers(const std::string &ip, const std::string &user, const std::string &pass, int port);
	void Join();
	void Stop();

	inline void SetNumberOfReceivers(size_t n) { m_nReceivers = n; }
	inline void SetNumberOfSavers(size_t n) { m_nSavers = n; }

public:
	Service();
	Service(const Service&) = delete;
	Service(Service&&) = delete;
	Service& operator=(const Service&) = delete;
	Service& operator=(Service&&) = delete;
	~Service();
};
