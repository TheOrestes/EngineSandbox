#include "sandboxPCH.h"
#include "SandboxEngine.h"
#include "Logger.h"

//---------------------------------------------------------------------------------------------------------------------
SandboxEngine::SandboxEngine()
{
}

//---------------------------------------------------------------------------------------------------------------------
SandboxEngine::~SandboxEngine()
{
}

//---------------------------------------------------------------------------------------------------------------------
void SandboxEngine::Run()
{
	std::cout << "Hello from Sandbox Engine!" << std::endl;

	LOG_CRITICAL("This is critical");
	LOG_DEBUG("This is debug");

	while (true)
	{

	}
}
