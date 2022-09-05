#include "pch.h"
#include "ConcurrencyTest.h"


int main()
{
    init_apartment();

	ConcurrencyTest cTest;
	cTest.Run();
}
