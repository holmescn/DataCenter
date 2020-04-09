#pragma once
#include <unordered_set>
#include <chrono>
#include <string>
#include <future>
#include <thread>
#include <taos.h>
#include "receiver.h"


class Saver
{
	enum class State {
		Initialized,
		Start,
		WaitTick,
		SaveTick,
		Error
	} m_state;

	std::thread m_thread;
	std::shared_future<void> m_fExit;
	std::unordered_set<std::string> m_knownStocks;

	RCV_REPORT_STRUCTEx m_rcvData;
	TAOS* m_taos;

	std::string m_taos_ip;
	std::string m_taos_user;
	std::string m_taos_pass;
	int m_taos_port;

	std::chrono::steady_clock::time_point m_timer;
	BufferQueue_t& m_bufferQueue;

public:
	Saver(BufferQueue_t& q, std::shared_future<void> fExit);
	Saver(const Saver&) = delete;
	Saver(Saver&&) = delete;
	Saver& operator=(const Saver&) = delete;
	Saver& operator=(Saver&&) = delete;
	~Saver();

	void ThreadFunc();

	inline void SetTDengineIP(const std::string& ip) { m_taos_ip = ip; }
	inline void SetTDengineUser(const std::string& user) { m_taos_user = user; }
	inline void SetTDenginePassword(const std::string& pass) { m_taos_pass = pass; }
	inline void SetTDenginePort(int port) { m_taos_port = port; }
	inline void Start() { m_state = State::Start; }

protected:
	void StateMachine();
	void Connect();
	void GetTickFromQueue();
	void SaveTick();
	bool CreateTable(const char *tableName);
	void InsertTick(const char* tableName);
};