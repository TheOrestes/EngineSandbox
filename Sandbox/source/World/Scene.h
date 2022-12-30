#pragma once

class VulkanContext;
class VulkanModel;
class Camera;
class UIManager;

//---------------------------------------------------------------------------------------------------------------------
class Scene
{
public:
	Scene();
	~Scene();

	bool							LoadScene(const VulkanContext* pContext);
	void							Cleanup(VulkanContext* pContext);

	void							Update(float dt);
	void							UpdateUniforms(const VulkanContext* pContext, uint32_t imageIndex);
	void							Render(const VulkanContext* pContext, uint32_t imageIndex);

public:
	inline VulkanModel*				GetFirstModel() const { return m_ListModels[0]; }
	inline Camera*					GetCamera()	const { return m_pCamera; }

private:
	bool							LoadModels(const VulkanContext* pContext);

private:
	std::vector <VulkanModel*>		m_ListModels;
	Camera*							m_pCamera;
public:
	UIManager*						m_pGUI;
};

