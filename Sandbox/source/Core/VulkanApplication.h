#pragma once

#include "IApplication.h"
#include "vulkan/vulkan.hpp"

class VulkanApplication : public IApplication
{
public:
	VulkanApplication();
	~VulkanApplication();

	virtual bool		Initialize(void* pWindow) override;
	virtual void		Update(float dt) override;
	virtual void		Render() override;
	virtual void		Destroy() override;

private:
	VulkanApplication(const VulkanApplication&);
	VulkanApplication& operator=(const VulkanApplication&);

	bool				CreateInstance();

private:
	VkInstance			m_vkInstance;

};

