#include "pch.h"
#include "ConcurrencyTest.h"
#include "WeakReferenceTest.h"

int main()
{
    init_apartment();

	ConcurrencyTest cTest;
	cTest.Run();

	WeakReferenceTest wTest;
	wTest.Run();
}
