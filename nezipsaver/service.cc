#include <cstdio>
#include <nng/protocol/reqrep0/rep.h>
#include "service.h"

static void fatal(const char* func, int rv)
{
	fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
	exit(1);
}

void Service::StartReceivers(const char* url)
{
	int rv;

	/*  Create the socket. */
	rv = nng_rep0_open(&m_sock);
	if (rv != 0) {
		fatal("nng_rep0_open", rv);
	}

	for (size_t i = 0; i < m_nReceivers; i++) {
		m_receivers.emplace_front(m_bufferQueue);
		Receiver& r = m_receivers.front();

		if ((rv = nng_aio_alloc(&r.aio, Receiver::aio_cb, &r)) != 0) {
			fatal("nng_aio_alloc", rv);
		}

		if ((rv = nng_ctx_open(&r.ctx, m_sock)) != 0) {
			fatal("nng_ctx_open", rv);
		}
	}

	if ((rv = nng_listen(m_sock, url, NULL, 0)) != 0) {
		fatal("nng_listen", rv);
	}

	for (auto &r : m_receivers) {
		// this starts them going (INIT state)
		Receiver::aio_cb(&r);
	}
}

void Service::StartSavers()
{
	for (size_t i = 0; i < m_nSavers; i++) {
		m_savers.emplace_front(m_bufferQueue, m_fExit);
	}
}

void Service::Join()
{
	using namespace std::literals::chrono_literals;
	while (m_fExit.wait_for(1s) == std::future_status::timeout);
}

void Service::Stop()
{
	m_pExit.set_value();
}

Service::Service()
	: m_nReceivers(64), m_nSavers(4)
{
	m_fExit = m_pExit.get_future();
}

Service::~Service()
{
}
