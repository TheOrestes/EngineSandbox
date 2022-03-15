#pragma once

#include "Core.h"
#include <iostream>

class DLL_API SandboxEngine
{
public:
	SandboxEngine();
	virtual ~SandboxEngine();
	
	virtual void Run();
};
