#pragma once

class IApplication
{
public:
	IApplication() {}
	virtual ~IApplication() {}

	virtual bool	Initialize(void* pWindow) = 0;
	virtual void	Update(float dt) = 0;
	virtual void	Render() = 0;
	virtual void	Cleanup() = 0;
};

