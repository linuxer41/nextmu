#include "stdafx.h"
#include "mu_modelrenderer.h"
#include "mu_skeletonmanager.h"
#include "mu_resourcesmanager.h"
#include "mu_terrain.h"
#include "mu_model.h"
#include "mu_skeletoninstance.h"
#include <glm/gtc/type_ptr.hpp>

bgfx::UniformHandle MUModelRenderer::TextureSampler = BGFX_INVALID_HANDLE;
bgfx::UniformHandle MUModelRenderer::Settings1Uniform = BGFX_INVALID_HANDLE;
bgfx::UniformHandle MUModelRenderer::TransformUniform = BGFX_INVALID_HANDLE;
bgfx::ProgramHandle MUModelRenderer::RenderProgram = BGFX_INVALID_HANDLE; // Temporary
const NTerrain *MUModelRenderer::Terrain = nullptr;

const mu_boolean MUModelRenderer::Initialize()
{
	TextureSampler = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
	if (bgfx::isValid(TextureSampler) == false)
	{
		return false;
	}

	Settings1Uniform = bgfx::createUniform("u_settings1", bgfx::UniformType::Vec4);
	if (bgfx::isValid(Settings1Uniform) == false)
	{
		return false;
	}

	TransformUniform = bgfx::createUniform("u_transform", bgfx::UniformType::Vec4);
	if (bgfx::isValid(TransformUniform) == false)
	{
		return false;
	}

	RenderProgram = MUResourcesManager::GetProgram("model_texture");
	if (bgfx::isValid(RenderProgram) == false)
	{
		return false;
	}

	return true;
}

void MUModelRenderer::Destroy()
{
	if (bgfx::isValid(TextureSampler))
	{
		bgfx::destroy(TextureSampler);
		TextureSampler = BGFX_INVALID_HANDLE;
	}

	if (bgfx::isValid(Settings1Uniform))
	{
		bgfx::destroy(Settings1Uniform);
		Settings1Uniform = BGFX_INVALID_HANDLE;
	}

	if (bgfx::isValid(TransformUniform))
	{
		bgfx::destroy(TransformUniform);
		TransformUniform = BGFX_INVALID_HANDLE;
	}

	if (bgfx::isValid(RenderProgram))
	{
		bgfx::destroy(RenderProgram);
		RenderProgram = BGFX_INVALID_HANDLE;
	}
}

void MUModelRenderer::AttachTerrain(NTerrain *terrain)
{
	Terrain = terrain;
}

void MUModelRenderer::RenderMesh(const NModel *model, const mu_uint32 bonesOffset, const mu_uint32 meshIndex, const glm::vec3 bodyOrigin, const mu_float bodyScale)
{
	const auto &mesh = model->Meshes[meshIndex];
	if (mesh.VertexBuffer.Count == 0) return;

	auto &texture = model->Textures[meshIndex];
	if (texture.Valid == false) return;

	switch (texture.Type)
	{
	case TextureType::Direct: bgfx::setTexture(0, TextureSampler, texture.Texture); break;
	}

	if (texture.HasAlpha)
	{
		bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_BLEND_ALPHA);
	}
	else
	{
		bgfx::setState(BGFX_STATE_DEFAULT);
	}
	
	bgfx::setTexture(1, MUSkeletonManager::GetSampler(), MUSkeletonManager::GetTexture());
	bgfx::setTexture(2, Terrain->GetLightmapSampler(), Terrain->GetLightmapTexture());

	glm::vec4 settings(static_cast<mu_float>(bonesOffset), 0.0f, 0.0f, 0.0f);
	bgfx::setUniform(Settings1Uniform, glm::value_ptr(settings));

	glm::vec4 transform(bodyOrigin, bodyScale);
	bgfx::setUniform(TransformUniform, glm::value_ptr(transform));

	bgfx::setVertexBuffer(0, model->VertexBuffer, mesh.VertexBuffer.Offset, mesh.VertexBuffer.Count);
	bgfx::submit(0, RenderProgram);
}

void MUModelRenderer::RenderBody(const NModel *model, const mu_uint32 bonesOffset, const glm::vec3 bodyOrigin, const mu_float bodyScale)
{
	const mu_uint32 numMeshes = static_cast<mu_uint32>(model->Meshes.size());
	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		RenderMesh(model, bonesOffset, m, bodyOrigin, bodyScale);
	}
}