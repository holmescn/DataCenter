#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/supplemental/util/platform.h>
#include <concurrentqueue.h>
#include "stockdrv.h"

constexpr auto WM_SENT_ONE_RECORD = WM_USER + 10;
typedef moodycamel::ConcurrentQueue<RCV_REPORT_STRUCTEx> BufferQueue_t;

struct Worker
{
	enum class State {
		Start,
		WaitData,
		PrepareSend,
		Send,
		Ack,
		Error
	} state;

	RCV_REPORT_STRUCTEx rcvData;
	BufferQueue_t& bufferQueue;

	nng_aio* aio;
	nng_msg* msg;
	nng_ctx  ctx;
	HWND &hWnd;

	Worker(BufferQueue_t& q, HWND &hWnd);
	Worker(const Worker&) = delete;
	Worker(Worker&&) = delete;
	Worker& operator=(const Worker&) = delete;
	Worker& operator=(Worker&&) = delete;
	~Worker();

	static void aio_cb(void* arg);
};
