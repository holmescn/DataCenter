#include "worker.h"

Worker::Worker(BufferQueue_t& q)
	: bufferQueue(q), msg(nullptr), state(State::Start)
{
	//
}

Worker::~Worker()
{
	nng_ctx_close(ctx);
	nng_aio_free(aio);
}

void Worker::aio_cb(void* arg)
{
	int rv;
	Worker* w = reinterpret_cast<Worker*>(arg);
	BufferQueue_t& q = w->bufferQueue;
	constexpr nng_duration _100_ms = 100;

	switch (w->state)
	{
	case State::Start:
		w->state = State::WaitData;
		nng_sleep_aio(1, w->aio);
		break;
	case State::WaitData:
		if (q.try_dequeue(w->rcvData)) {
			w->state = State::Send;
		}
		nng_sleep_aio(1, w->aio);
		break;
	case State::Send:
		if ((rv = nng_msg_alloc(&w->msg, sizeof w->rcvData)) == ENOMEM) {
			w->msg = nullptr;
			nng_sleep_aio(_100_ms, w->aio);
		}
		else {
			void* body = nng_msg_body(w->msg);
			memcpy(body, &w->rcvData, sizeof w->rcvData);
			nng_aio_set_msg(w->aio, w->msg);
			w->state = State::Sent;
			nng_ctx_send(w->ctx, w->aio);
		}
		break;
	case State::Sent:
		rv = nng_aio_result(w->aio);
		if (rv == 0) {
			w->msg = nullptr;
			w->state = State::Ack;
			nng_ctx_recv(w->ctx, w->aio);
		}
		else {
			// TODO handle errors: TIMEOUT, CANCELED
			w->state = State::Error;
		}
		break;
	case State::Ack:
		rv = nng_aio_result(w->aio);
		if (rv == 0) {
			nng_msg *msg = nng_aio_get_msg(w->aio);
			size_t bodyLen = nng_msg_len(msg);
			char* body = reinterpret_cast<char*>(nng_msg_body(msg));
			if (bodyLen >= 3 && memcmp(body, "ACK", 3) == 0) {
				w->state = State::WaitData;
				nng_sleep_aio(1, w->aio);
				// TODO notify main window
			}
			else {
				// TODO bad response: bad response
				w->state = State::Error;
			}
			nng_msg_free(msg);
		}
		else {
			// TODO handle errors: TIMEOUT, CANCELED
			w->state = State::Error;
		}
		break;
	case State::Error:
		break;
	default:
		break;
	}
}
