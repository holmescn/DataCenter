#include <iomanip>
#include <sstream>
#include <spdlog/spdlog.h>
#include "saver.h"

Saver::Saver(BufferQueue_t& q, std::shared_future<void> fExit)
	: m_state(State::Start)
	, m_thread(&Saver::ThreadFunc, this)
	, m_fExit(fExit)
	, m_bufferQueue(q)
	, m_taos(nullptr)
	, m_taos_port(6030)
{
	m_timer = std::chrono::steady_clock::now();	
}

Saver::~Saver()
{
	if (m_taos) {
		taos_close(m_taos);
	}
}

void Saver::ThreadFunc()
{
	using namespace std::literals::chrono_literals;
	do {
		StateMachine();
	} while (m_fExit.wait_for(100ms) == std::future_status::timeout);

	// TODO Clean up.
}

void Saver::StateMachine()
{
	switch (m_state)
	{
	case State::Initialized:
		break;
	case Saver::State::Start:
		Connect();
		break;
	case State::WaitTick:
		GetTickFromQueue();
		break;
	case State::SaveTick:
		SaveTick();
		break;
	case State::Error:
		break;
	default:
		break;
	}
}

void Saver::Connect()
{
	using namespace std::chrono_literals;

	if (std::chrono::steady_clock::now() < m_timer) {
		return;
	}

	if (m_taos) {
		spdlog::info("Close previous TDengine connection.");
		taos_close(m_taos);
	}

	char* ip = const_cast<char*>(m_taos_ip.c_str());
	char* user = const_cast<char*>(m_taos_user.c_str());
	char* pass = const_cast<char*>(m_taos_pass.c_str());

	spdlog::info("Connect to TDengine at {}:{} with user {}.", m_taos_ip, m_taos_port, m_taos_user);
	m_taos = taos_connect(ip, user, pass, "stock", m_taos_port);

	if (m_taos) {
		spdlog::info("Connect to TDengine at {}:{} success.", m_taos_ip, m_taos_port);
		m_state = State::WaitTick;
	}
	else {
		spdlog::info("Connect to TDengine at {}:{} failed: {}", m_taos_ip, m_taos_port, taos_errstr(m_taos));
		m_timer = std::chrono::steady_clock::now() + 5s;
	}
}

void Saver::GetTickFromQueue()
{
	if (m_bufferQueue.try_dequeue(m_rcvData)) {
		m_state = State::SaveTick;
	}
}

void Saver::SaveTick()
{
	using namespace std::chrono_literals;

	if (std::chrono::steady_clock::now() < m_timer) {
		return;
	}

	auto& d = m_rcvData;
	char* tableName = reinterpret_cast<char*>(&d.m_wMarket);
	if (m_knownStocks.find(tableName) == m_knownStocks.end()) {
		if (!CreateTable(tableName)) {
			m_state = State::Error;
			return;
		}
	}

	InsertTick(tableName);
	m_state = State::WaitTick;
}

bool Saver::CreateTable(const char* tableName)
{
	std::string sql;
	sql = "CREATE TABLE IF NOT EXISTS ";
	sql += tableName;
	sql += "(ts TIMESTAMP,"
		"priceLastClose FLOAT,"
		"priceOpen FLOAT,"
		"priceHigh FLOAT,"
		"priceLow FLOAT,"
		"priceLast FLOAT,"
		"volume FLOAT,"
		"amount FLOAT,"
		"bid1p FLOAT,"
		"bid1v FLOAT,"
		"bid2p FLOAT,"
		"bid2v FLOAT,"
		"bid3p FLOAT,"
		"bid3v FLOAT,"
		"bid4p FLOAT,"
		"bid4v FLOAT,"
		"bid5p FLOAT,"
		"bid5v FLOAT,"
		"ask1p FLOAT,"
		"ask1v FLOAT,"
		"ask2p FLOAT,"
		"ask2v FLOAT,"
		"ask3p FLOAT,"
		"ask3v FLOAT,"
		"ask4p FLOAT,"
		"ask4v FLOAT,"
		"ask5p FLOAT,"
		"ask5v FLOAT)";

	if (taos_query(m_taos, sql.c_str()) == -1) {
		int _errno = taos_errno(m_taos);
		return false;
	}

	m_knownStocks.insert(tableName);
	return true;
}

void Saver::InsertTick(const char* tableName)
{
	auto& d = m_rcvData;
	std::stringstream ss;
	ss << "INSERT INTO " << tableName << " VALUES (";
	ss << d.m_time * 1000LL << ",";
	ss << std::setprecision(4)
		<< d.m_fLastClose << ","
		<< d.m_fOpen << ","
		<< d.m_fHigh << ","
		<< d.m_fLow << ","
		<< d.m_fNewPrice << ",";
	ss << std::setprecision(8)
		<< d.m_fVolume << ","
		<< d.m_fAmount << ",";

	ss << std::setprecision(4)
		<< d.m_fBuyPrice[0] << ","
		<< std::setprecision(8)
		<< d.m_fBuyVolume[0] << ",";

	ss << std::setprecision(4)
		<< d.m_fBuyPrice[1] << ","
		<< std::setprecision(8)
		<< d.m_fBuyVolume[1] << ",";

	ss << std::setprecision(4)
		<< d.m_fBuyPrice[2] << ","
		<< std::setprecision(8)
		<< d.m_fBuyVolume[2] << ",";

	ss << std::setprecision(4)
		<< d.m_fBuyPrice4 << ","
		<< std::setprecision(8)
		<< d.m_fBuyVolume4 << ",";

	ss << std::setprecision(4)
		<< d.m_fBuyPrice5 << ","
		<< std::setprecision(8)
		<< d.m_fBuyVolume5 << ",";

	ss << std::setprecision(4)
		<< d.m_fSellPrice[0] << ","
		<< std::setprecision(8)
		<< d.m_fSellVolume[0] << ",";

	ss << std::setprecision(4)
		<< d.m_fSellPrice[1] << ","
		<< std::setprecision(8)
		<< d.m_fSellVolume[1] << ",";

	ss << std::setprecision(4)
		<< d.m_fSellPrice[2] << ","
		<< std::setprecision(8)
		<< d.m_fSellVolume[2] << ",";

	ss << std::setprecision(4)
		<< d.m_fSellPrice4 << ","
		<< std::setprecision(8)
		<< d.m_fSellVolume4 << ",";

	ss << std::setprecision(4)
		<< d.m_fSellPrice5 << ","
		<< std::setprecision(8)
		<< d.m_fSellVolume5;

	ss << ")";
	std::string sql = ss.str();

	if (taos_query(m_taos, sql.c_str()) == -1) {
		spdlog::error("INSERT failed: {}", taos_errstr(m_taos));
	}
}
