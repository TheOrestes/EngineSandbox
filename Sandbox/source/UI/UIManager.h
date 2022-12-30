#pragma once

class VulkanContext;

class UIManager
{
public:
	UIManager();
	~UIManager();

	bool			Initialize(const VulkanContext* pContext);

	void			HandleWindowResize(VulkanContext* pContext);
	void			BeginRender(const VulkanContext* pContext);
	void			EndRender(const VulkanContext* pContext, uint32_t imageIndex);
	void			Render(const VulkanContext* pContext);
};

