///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Windows WIC (built-in) image loading (JPG/PNG/etc)
#include <windows.h>
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

namespace
{
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName = "bUseTexture";
    const char* g_UseLightingName = "bUseLighting";

    // Helper: convert UTF-8 char* to wide string
    std::wstring ToWide(const char* s)
    {
        if (!s) return L"";
        int len = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
        if (len <= 0) return L"";
        std::wstring out;
        out.resize((size_t)len);
        MultiByteToWideChar(CP_UTF8, 0, s, -1, &out[0], len);
        // remove trailing null
        if (!out.empty() && out.back() == L'\0') out.pop_back();
        return out;
    }

    // Helper: WIC load into RGBA8
    bool LoadImageRGBA_WIC(const char* filename, std::vector<unsigned char>& rgba, int& width, int& height)
    {
        width = 0;
        height = 0;
        rgba.clear();

        std::wstring wpath = ToWide(filename);
        if (wpath.empty())
        {
            std::cout << "WIC: invalid filename.\n";
            return false;
        }

        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        bool didInit = SUCCEEDED(hr);

        IWICImagingFactory* factory = nullptr;
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&factory));
        if (FAILED(hr) || !factory)
        {
            std::cout << "WIC: failed to create factory.\n";
            if (didInit) CoUninitialize();
            return false;
        }

        IWICBitmapDecoder* decoder = nullptr;
        hr = factory->CreateDecoderFromFilename(wpath.c_str(), nullptr, GENERIC_READ,
            WICDecodeMetadataCacheOnLoad, &decoder);
        if (FAILED(hr) || !decoder)
        {
            std::cout << "WIC: failed to open image: " << filename << "\n";
            factory->Release();
            if (didInit) CoUninitialize();
            return false;
        }

        IWICBitmapFrameDecode* frame = nullptr;
        hr = decoder->GetFrame(0, &frame);
        if (FAILED(hr) || !frame)
        {
            std::cout << "WIC: failed to get frame: " << filename << "\n";
            decoder->Release();
            factory->Release();
            if (didInit) CoUninitialize();
            return false;
        }

        UINT w = 0, h = 0;
        frame->GetSize(&w, &h);
        width = (int)w;
        height = (int)h;

        IWICFormatConverter* converter = nullptr;
        hr = factory->CreateFormatConverter(&converter);
        if (FAILED(hr) || !converter)
        {
            std::cout << "WIC: failed converter.\n";
            frame->Release();
            decoder->Release();
            factory->Release();
            if (didInit) CoUninitialize();
            return false;
        }

        hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone, nullptr, 0.0,
            WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
        {
            std::cout << "WIC: failed to convert to RGBA: " << filename << "\n";
            converter->Release();
            frame->Release();
            decoder->Release();
            factory->Release();
            if (didInit) CoUninitialize();
            return false;
        }

        rgba.resize((size_t)width * (size_t)height * 4u);
        hr = converter->CopyPixels(nullptr, (UINT)(width * 4), (UINT)rgba.size(), rgba.data());
        if (FAILED(hr))
        {
            std::cout << "WIC: CopyPixels failed: " << filename << "\n";
            rgba.clear();
            converter->Release();
            frame->Release();
            decoder->Release();
            factory->Release();
            if (didInit) CoUninitialize();
            return false;
        }

        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        if (didInit) CoUninitialize();
        return true;
    }
}

SceneManager::SceneManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes = new ShapeMeshes();
    m_loadedTextures = 0;
}

SceneManager::~SceneManager()
{
    DestroyGLTextures();

    m_pShaderManager = NULL;
    if (NULL != m_basicMeshes)
    {
        delete m_basicMeshes;
        m_basicMeshes = NULL;
    }
}

bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
    int width = 0;
    int height = 0;
    std::vector<unsigned char> rgba;

    if (!LoadImageRGBA_WIC(filename, rgba, width, height))
    {
        std::cout << "Could not load image: " << filename << std::endl;
        return false;
    }

    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_textureIDs[m_loadedTextures].ID = textureID;
    m_textureIDs[m_loadedTextures].tag = tag;
    m_loadedTextures++;

    std::cout << "Loaded texture: " << filename << " as tag: " << tag << std::endl;
    return true;
}

void SceneManager::BindGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}

void SceneManager::DestroyGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        glDeleteTextures(1, &m_textureIDs[i].ID);
    }
    m_loadedTextures = 0;
}

int SceneManager::FindTextureID(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        if (m_textureIDs[i].tag == tag)
            return (int)m_textureIDs[i].ID;
    }
    return -1;
}

int SceneManager::FindTextureSlot(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        if (m_textureIDs[i].tag == tag)
            return i;
    }
    return -1;
}

bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
    for (size_t i = 0; i < m_objectMaterials.size(); i++)
    {
        if (m_objectMaterials[i].tag == tag)
        {
            material = m_objectMaterials[i];
            return true;
        }
    }
    return false;
}

void SceneManager::SetTransformations(
    glm::vec3 scaleXYZ,
    float XrotationDegrees,
    float YrotationDegrees,
    float ZrotationDegrees,
    glm::vec3 positionXYZ,
    glm::vec3 offset)
{
    // Correct GLM overloads (no experimental headers needed)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, positionXYZ + offset);
    model = glm::rotate(model, glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scaleXYZ);

    if (NULL != m_pShaderManager)
        m_pShaderManager->setMat4Value(g_ModelName, model);
}

void SceneManager::SetShaderColor(float r, float g, float b, float a)
{
    glm::vec4 currentColor(r, g, b, a);
    if (NULL != m_pShaderManager)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
    }
}

void SceneManager::SetShaderTexture(std::string textureTag)
{
    if (NULL == m_pShaderManager) return;

    int textureID = FindTextureID(textureTag);
    int slot = FindTextureSlot(textureTag);

    if (textureID >= 0 && slot >= 0)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, true);
        m_pShaderManager->setSampler2DValue(g_TextureValueName, slot);
    }
    else
    {
        m_pShaderManager->setIntValue(g_UseTextureName, false);
    }
}

void SceneManager::SetTextureUVScale(float u, float v)
{
    if (NULL != m_pShaderManager)
        m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
}

void SceneManager::SetShaderMaterial(std::string materialTag)
{
    if (NULL == m_pShaderManager) return;

    OBJECT_MATERIAL material;
    if (FindMaterial(materialTag, material))
    {
        m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
        m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
        m_pShaderManager->setFloatValue("material.shininess", material.shininess);
    }
}

void SceneManager::SetupSceneLights()
{
    if (NULL == m_pShaderManager) return;

    m_pShaderManager->setIntValue(g_UseLightingName, true);

    m_pShaderManager->setVec3Value("directionalLight.direction", glm::vec3(-0.35f, -1.0f, -0.25f));
    m_pShaderManager->setVec3Value("directionalLight.ambient", glm::vec3(0.20f, 0.20f, 0.20f));
    m_pShaderManager->setVec3Value("directionalLight.diffuse", glm::vec3(0.75f, 0.75f, 0.70f));
    m_pShaderManager->setVec3Value("directionalLight.specular", glm::vec3(0.90f, 0.90f, 0.90f));
    m_pShaderManager->setIntValue("directionalLight.bActive", true);

    m_pShaderManager->setVec3Value("pointLights[0].position", glm::vec3(10.0f, 12.0f, 10.0f));
    m_pShaderManager->setVec3Value("pointLights[0].ambient", glm::vec3(0.10f, 0.10f, 0.10f));
    m_pShaderManager->setVec3Value("pointLights[0].diffuse", glm::vec3(0.85f, 0.82f, 0.75f));
    m_pShaderManager->setVec3Value("pointLights[0].specular", glm::vec3(1.00f, 1.00f, 1.00f));
    m_pShaderManager->setIntValue("pointLights[0].bActive", true);

    m_pShaderManager->setVec3Value("pointLights[1].position", glm::vec3(-14.0f, 8.0f, -6.0f));
    m_pShaderManager->setVec3Value("pointLights[1].ambient", glm::vec3(0.05f, 0.07f, 0.10f));
    m_pShaderManager->setVec3Value("pointLights[1].diffuse", glm::vec3(0.30f, 0.50f, 0.95f));
    m_pShaderManager->setVec3Value("pointLights[1].specular", glm::vec3(0.60f, 0.70f, 0.90f));
    m_pShaderManager->setIntValue("pointLights[1].bActive", true);

    for (int i = 2; i < 5; i++)
    {
        std::string idx = std::to_string(i);
        m_pShaderManager->setIntValue("pointLights[" + idx + "].bActive", false);
    }

    m_pShaderManager->setIntValue("spotLight.bActive", false);
}

void SceneManager::LoadSceneTextures()
{
    CreateGLTexture("textures/natural-stone-aged-pavement.jpg", "stone");
    CreateGLTexture("textures/view-evergreen-boxwood-shrubs.jpg", "hedgeLight");
    CreateGLTexture("textures/dark-green-leaves-textured.jpg", "hedgeDark");

    BindGLTextures();
}

void SceneManager::PrepareScene()
{
    LoadSceneTextures();

    OBJECT_MATERIAL matte;
    matte.tag = "matte";
    matte.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
    matte.specularColor = glm::vec3(0.15f, 0.15f, 0.15f);
    matte.shininess = 8.0f;
    m_objectMaterials.push_back(matte);

    OBJECT_MATERIAL glossy;
    glossy.tag = "glossy";
    glossy.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glossy.specularColor = glm::vec3(0.55f, 0.55f, 0.55f);
    glossy.shininess = 48.0f;
    m_objectMaterials.push_back(glossy);

    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadConeMesh();
    m_basicMeshes->LoadTaperedCylinderMesh();
    m_basicMeshes->LoadSphereMesh();
    m_basicMeshes->LoadTorusMesh();

    SetupSceneLights();
}

void SceneManager::RenderScene()
{
    glm::vec3 scaleXYZ;
    glm::vec3 positionXYZ;

    if (NULL != m_pShaderManager) { m_pShaderManager->setIntValue(g_UseLightingName, true); }
    SetShaderMaterial("matte");

    // Ground
    scaleXYZ = glm::vec3(40.0f, 1.0f, 40.0f);
    positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("stone");
    SetTextureUVScale(18.0f, 18.0f);
    m_basicMeshes->DrawPlaneMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // Pathway
    scaleXYZ = glm::vec3(14.0f, 1.0f, 24.0f);
    positionXYZ = glm::vec3(0.0f, 0.02f, 4.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("stone");
    SetTextureUVScale(10.0f, 16.0f);
    m_basicMeshes->DrawPlaneMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // Sky backdrop (no lighting)
    scaleXYZ = glm::vec3(45.0f, 1.0f, 25.0f);
    positionXYZ = glm::vec3(0.0f, 14.0f, -18.0f);
    SetTransformations(scaleXYZ, 90.0f, 0.0f, 0.0f, positionXYZ);
    if (NULL != m_pShaderManager) { m_pShaderManager->setIntValue(g_UseLightingName, false); }
    SetShaderColor(0.70f, 0.85f, 0.95f, 1.0f);
    m_basicMeshes->DrawPlaneMesh();
    if (NULL != m_pShaderManager) { m_pShaderManager->setIntValue(g_UseLightingName, true); }

    // Hedge maze walls (boxes)
    SetShaderMaterial("matte");
    SetShaderTexture("hedgeDark");

    scaleXYZ = glm::vec3(18.0f, 2.0f, 1.2f);
    positionXYZ = glm::vec3(0.0f, 1.0f, 8.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetTextureUVScale(6.0f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    scaleXYZ = glm::vec3(1.2f, 2.0f, 14.0f);
    positionXYZ = glm::vec3(-8.4f, 1.0f, 1.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetTextureUVScale(1.0f, 4.0f);
    m_basicMeshes->DrawBoxMesh();

    SetShaderTexture("hedgeLight");
    scaleXYZ = glm::vec3(12.0f, 2.0f, 1.2f);
    positionXYZ = glm::vec3(-2.0f, 1.0f, 1.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetTextureUVScale(4.0f, 1.0f);
    m_basicMeshes->DrawBoxMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // Cone hedges
    SetShaderTexture("hedgeLight");

    scaleXYZ = glm::vec3(4.0f, 10.0f, 4.0f);
    positionXYZ = glm::vec3(-12.0f, 5.0f, -2.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetTextureUVScale(2.0f, 3.0f);
    m_basicMeshes->DrawConeMesh();

    scaleXYZ = glm::vec3(5.5f, 12.0f, 5.5f);
    positionXYZ = glm::vec3(14.0f, 6.0f, -4.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetTextureUVScale(2.0f, 3.0f);
    m_basicMeshes->DrawConeMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // Spheres (bushes)
    scaleXYZ = glm::vec3(3.0f, 3.0f, 3.0f);
    positionXYZ = glm::vec3(-15.0f, 1.5f, 10.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetTextureUVScale(2.0f, 2.0f);
    m_basicMeshes->DrawSphereMesh();

    scaleXYZ = glm::vec3(4.2f, 4.2f, 4.2f);
    positionXYZ = glm::vec3(15.0f, 2.1f, 12.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetTextureUVScale(2.0f, 2.0f);
    m_basicMeshes->DrawSphereMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // Center hedge ring (cylinder)
    SetShaderTexture("hedgeDark");
    scaleXYZ = glm::vec3(10.0f, 2.0f, 10.0f);
    positionXYZ = glm::vec3(0.0f, 1.0f, -2.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetTextureUVScale(4.0f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // Spiral topiary: cylinder + torus rings
    SetShaderMaterial("glossy");
    SetShaderTexture("hedgeDark");

    scaleXYZ = glm::vec3(2.0f, 10.0f, 2.0f);
    positionXYZ = glm::vec3(0.0f, 6.0f, -2.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetTextureUVScale(2.0f, 4.0f);
    m_basicMeshes->DrawCylinderMesh();
    SetTextureUVScale(1.0f, 1.0f);

    for (int i = 0; i < 6; i++)
    {
        float y = 3.0f + (float)i * 1.4f;
        float rotY = (float)i * 25.0f;

        scaleXYZ = glm::vec3(2.6f, 0.7f, 2.6f);
        positionXYZ = glm::vec3(0.0f, y, -2.0f);

        SetTransformations(scaleXYZ, 0.0f, rotY, 0.0f, positionXYZ);
        SetTextureUVScale(2.0f, 2.0f);
        m_basicMeshes->DrawTorusMesh();
    }
    SetTextureUVScale(1.0f, 1.0f);

    // Tiered topiary: trunk + stacked tiers
    SetShaderMaterial("glossy");
    SetShaderColor(0.25f, 0.17f, 0.10f, 1.0f);

    scaleXYZ = glm::vec3(0.7f, 6.0f, 0.7f);
    positionXYZ = glm::vec3(12.5f, 3.0f, 7.5f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    m_basicMeshes->DrawCylinderMesh();

    SetShaderMaterial("matte");
    SetShaderTexture("hedgeLight");

    for (int i = 0; i < 4; i++)
    {
        float y = 2.0f + (float)i * 2.0f;
        float rr = 3.2f - (float)i * 0.5f;

        scaleXYZ = glm::vec3(rr, 1.1f, rr);
        positionXYZ = glm::vec3(12.5f, y, 7.5f);

        SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
        SetTextureUVScale(2.0f, 1.0f);
        m_basicMeshes->DrawCylinderMesh();
    }
    SetTextureUVScale(1.0f, 1.0f);
}