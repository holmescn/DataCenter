#pragma once
#include <nng/nng.h>
#include "stockdrv.h"
#include <concurrentqueue.h>

typedef moodycamel::ConcurrentQueue<RCV_REPORT_STRUCTEx> BufferQueue_t;

struct Receiver
{
	enum class State {
		Start,
		RecvData,
		SendAck
	} state;

	nng_aio* aio;
	nng_ctx  ctx;

	BufferQueue_t& bufferQueue;

	Receiver(BufferQueue_t& q);
	Receiver(const Receiver&) = delete;
	Receiver(Receiver&&) = delete;
	Receiver& operator=(const Receiver&) = delete;
	Receiver& operator=(Receiver&&) = delete;
	~Receiver();

	static void aio_cb(void* arg);
	bool EnqueueMessage(nng_msg* msg);
	void SendAck();
};
