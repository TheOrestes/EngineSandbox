#pragma once

#include "Core.h"

class DLL_API SandboxEngine
{
public:
	SandboxEngine();
	virtual ~SandboxEngine();
	
	virtual void Run();
};
