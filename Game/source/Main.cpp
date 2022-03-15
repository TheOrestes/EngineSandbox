
#include "Core/EngineInclude.h"

class MainGame : public SandboxEngine
{
public:
	MainGame() {};
	~MainGame() {};
};

int main(int argc, char** argv)
{
	MainGame Game;

	Game.Run();

	return 0;
}