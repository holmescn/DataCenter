#include <csignal>
#include "service.h"

Service* g_pService = nullptr;


int main(int argc, char** argv)
{
	Service s;
	g_pService = &s;
	s.StartReceivers("tcp://0.0.0.0:8001");
	// s.StartSavers();
	s.Join();
	return 0;
}
