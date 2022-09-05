#pragma once
#include "pch.h"
#include <iostream>

class ISample
{
public:
	void Run();
	void PrintIt(const char* msg)
	{		
		std::wcout << msg << std::endl;
	}
};

