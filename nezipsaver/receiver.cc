#include <cstring>
#include <spdlog/spdlog.h>
#include "receiver.h"

static void fatal(const char* func, int rv)
{
	fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
	exit(1);
}

Receiver::Receiver(BufferQueue_t& q)
	: bufferQueue(q), state(State::Start)
{

}

Receiver::~Receiver()
{
	//
}

void Receiver::aio_cb(void* arg)
{
	int rv;
	Receiver* w = reinterpret_cast<Receiver*>(arg);
	nng_msg* msg;

	switch (w->state)
	{
	case State::Start:
		w->state = State::RecvData;
		nng_ctx_recv(w->ctx, w->aio);
		break;
	case State::RecvData:
		if ((rv = nng_aio_result(w->aio)) == 0) {
			msg = nng_aio_get_msg(w->aio);
			if (w->EnqueueMessage(msg)) {
				w->SendAck();
				// spdlog::info("Received and enqueued a tick, ack.");
			}
		}
		else {
			fatal("nng_ctx_recv", rv);
		}
		break;
	case State::SendAck:
		if ((rv = nng_aio_result(w->aio)) == 0) {
			w->state = State::RecvData;
			nng_ctx_recv(w->ctx, w->aio);
			// spdlog::info("Ack is sent, back to recv data.");
		}
		else {
			fatal("nng_ctx_send", rv);
		}
		break;
	default:
		break;
	}
}

bool Receiver::EnqueueMessage(nng_msg* msg)
{
	bool isValidMessage = false;
	size_t bodySize = nng_msg_len(msg);
	RCV_REPORT_STRUCTEx rcvData;
	if (bodySize == sizeof rcvData) {
		void* body = nng_msg_body(msg);
		memcpy(&rcvData, body, sizeof rcvData);
		bufferQueue.enqueue(rcvData);

		isValidMessage = true;
	}
	else {
		// TODO invalid message.
		nng_ctx_recv(this->ctx, this->aio);
		isValidMessage = false;
		spdlog::error("Received invalid tick message with bodySize = {}", bodySize);
	}

	nng_msg_free(msg);
	return isValidMessage;
}

void Receiver::SendAck()
{
	constexpr size_t MESSAGE_SIZE = 4;
	nng_msg* msg = nullptr;
	int rv;

	if ((rv = nng_msg_alloc(&msg, MESSAGE_SIZE)) == 0) {
		void* body = nng_msg_body(msg);
		memcpy(body, "ACK", MESSAGE_SIZE);

		this->state = State::SendAck;
		nng_aio_set_msg(this->aio, msg);
		nng_ctx_send(this->ctx, this->aio);
	}
	else {
		fatal("nng_msg_alloc", rv);
	}
}
