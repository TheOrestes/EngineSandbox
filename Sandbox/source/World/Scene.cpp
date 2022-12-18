#include "sandboxPCH.h"
#include "Scene.h"

#include "Renderer/VulkanContext.h"
#include "Renderables/VulkanModel.h"
#include "UI/UIManager.h"
#include "Camera.h"
#include "Core/Core.h"

//-----------------------------------------------------------------------------------------------------------------------
Scene::Scene()
{
	m_pCamera = nullptr;
	m_pGUI = nullptr;
	m_ListModels.clear();
}

//-----------------------------------------------------------------------------------------------------------------------
Scene::~Scene()
{
	SAFE_DELETE(m_pCamera);
	SAFE_DELETE(m_pGUI);
	m_ListModels.clear();
}

//-----------------------------------------------------------------------------------------------------------------------
bool Scene::LoadScene(const VulkanContext* pContext)
{
	m_pCamera = new Camera();
	CHECK(LoadModels(pContext));

	m_pGUI = new UIManager();
	CHECK(m_pGUI->Initialize(pContext));
}

//-----------------------------------------------------------------------------------------------------------------------
void Scene::Cleanup(VulkanContext* pContext)
{
	for (VulkanModel* model : m_ListModels)
	{
		model->Cleanup(pContext); 
	}
}

//-----------------------------------------------------------------------------------------------------------------------
void Scene::Update(float dt)
{
	m_pCamera->Update(dt);

	for (VulkanModel* model : m_ListModels)
	{
		if (model != nullptr)
		{
			model->Update(m_pCamera, dt);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------
void Scene::UpdateUniforms(const VulkanContext* pContext, uint32_t imageIndex)
{
	for (VulkanModel* model : m_ListModels)
	{
		if (model != nullptr)
		{
			model->UpdateUniforms(pContext, imageIndex);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------
void Scene::Render(const VulkanContext* pContext, uint32_t imageIndex)
{
	for (VulkanModel* model : m_ListModels)
	{
		if (model != nullptr)
		{
			model->Render(pContext, imageIndex);
		}
	}

	m_pGUI->BeginRender(pContext);
	m_pGUI->Render(pContext);
	m_pGUI->EndRender(pContext, imageIndex);
}

//-----------------------------------------------------------------------------------------------------------------------
bool Scene::LoadModels(const VulkanContext* pContext)
{
	// Load Robot Model
	VulkanModel* pRobotModel = new VulkanModel();
	pRobotModel->LoadModel(pContext, "Gizmo/Gizmo.fbx");
	pRobotModel->m_vecPosition = glm::vec3(0, 0, 0);
	pRobotModel->m_vecScale = glm::vec3(0.1f);
	pRobotModel->m_bUpdate = true;
	pRobotModel->SetupDescriptors(pContext);
	

	m_ListModels.push_back(pRobotModel);

	// Load Robot Model
	VulkanModel* pBarbModel = new VulkanModel();
	pBarbModel->LoadModel(pContext, "Barbarian/BarbNew.fbx");
	pBarbModel->m_vecPosition = glm::vec3(0, 0, 0);
	pBarbModel->m_vecScale = glm::vec3(0.1f);
	pBarbModel->m_bUpdate = false;
	pBarbModel->SetupDescriptors(pContext);

	m_ListModels.push_back(pBarbModel);

	return true;
}
