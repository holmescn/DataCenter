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

}

void Receiver::aio_cb(void* arg)
{
	int rv;
	Receiver* w = reinterpret_cast<Receiver*>(arg);
	nng_msg* msg;

	switch (w->state)
	{
	case State::Start:
		w->state = State::Recv;
		nng_ctx_recv(w->ctx, w->aio);
		break;
	case State::Recv:
		if ((rv = nng_aio_result(w->aio)) == 0) {
			msg = nng_aio_get_msg(w->aio);
			w->EnqueueMessage(msg);
		}
		else {
			fatal("nng_ctx_recv", rv);
		}
		break;
	case State::Send:
		if ((rv = nng_aio_result(w->aio)) == 0) {
			w->state = State::Recv;
			nng_ctx_recv(w->ctx, w->aio);
		}
		else {
			fatal("nng_ctx_send", rv);
		}
		break;
	default:
		break;
	}
}

void Receiver::EnqueueMessage(nng_msg* msg)
{
	size_t bodySize = nng_msg_len(msg);
	RCV_REPORT_STRUCTEx rcvData;
	if (bodySize == sizeof rcvData) {
		void* body = nng_msg_body(msg);
		memcpy(&rcvData, body, sizeof rcvData);
		bufferQueue.enqueue(rcvData);

		SendAck();
	}
	else {
		// TODO invalid message.
		nng_ctx_recv(this->ctx, this->aio);
	}

	nng_msg_free(msg);
}

void Receiver::SendAck()
{
	constexpr size_t MESSAGE_SIZE = 4;
	nng_msg* msg = nullptr;
	int rv;

	if ((rv = nng_msg_alloc(&msg, MESSAGE_SIZE)) == NNG_ENOMEM) {
		fatal("nng_msg_alloc", rv);
	}
	else {
		void* body = nng_msg_body(msg);
		memset(body, 0, MESSAGE_SIZE);
		memcpy(body, "ACK", MESSAGE_SIZE);

		this->state = State::Send;
		nng_aio_set_msg(this->aio, msg);
		nng_ctx_send(this->ctx, this->aio);
	}
}
