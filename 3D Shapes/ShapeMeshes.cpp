#include "ShapeMeshes.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <cmath>
#include <vector>

static constexpr float PI = 3.14159265358979323846f;

// Vertex format: position(3), normal(3), uv(2) = 8 floats
static inline void PushVertex(std::vector<float>& verts,
    const glm::vec3& p,
    const glm::vec3& n,
    const glm::vec2& uv)
{
    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
    verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z);
    verts.push_back(uv.x); verts.push_back(uv.y);
}

static void UploadIndexed(GLuint& vao, GLuint& vbo, GLuint& ebo,
    const std::vector<float>& verts,
    const std::vector<unsigned int>& idx)
{
    if (vao != 0) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo != 0) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (ebo != 0) { glDeleteBuffers(1, &ebo); ebo = 0; }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(idx.size() * sizeof(unsigned int)), idx.data(), GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

static void UploadNonIndexed(GLuint& vao, GLuint& vbo,
    const std::vector<float>& verts)
{
    if (vao != 0) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo != 0) { glDeleteBuffers(1, &vbo); vbo = 0; }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

ShapeMeshes::ShapeMeshes()
    : m_planeVAO(0), m_planeVBO(0),
    m_boxVAO(0), m_boxVBO(0),
    m_cylVAO(0), m_cylVBO(0), m_cylEBO(0), m_cylIndexCount(0),
    m_coneVAO(0), m_coneVBO(0), m_coneEBO(0), m_coneIndexCount(0),
    m_sphereVAO(0), m_sphereVBO(0), m_sphereEBO(0), m_sphereIndexCount(0),
    m_torusVAO(0), m_torusVBO(0), m_torusEBO(0), m_torusIndexCount(0),
    m_taperVAO(0), m_taperVBO(0), m_taperEBO(0), m_taperIndexCount(0)
{
}

ShapeMeshes::~ShapeMeshes()
{
    // plane
    if (m_planeVAO) glDeleteVertexArrays(1, &m_planeVAO);
    if (m_planeVBO) glDeleteBuffers(1, &m_planeVBO);

    // box
    if (m_boxVAO) glDeleteVertexArrays(1, &m_boxVAO);
    if (m_boxVBO) glDeleteBuffers(1, &m_boxVBO);

    // cylinder
    if (m_cylVAO) glDeleteVertexArrays(1, &m_cylVAO);
    if (m_cylVBO) glDeleteBuffers(1, &m_cylVBO);
    if (m_cylEBO) glDeleteBuffers(1, &m_cylEBO);

    // cone
    if (m_coneVAO) glDeleteVertexArrays(1, &m_coneVAO);
    if (m_coneVBO) glDeleteBuffers(1, &m_coneVBO);
    if (m_coneEBO) glDeleteBuffers(1, &m_coneEBO);

    // sphere
    if (m_sphereVAO) glDeleteVertexArrays(1, &m_sphereVAO);
    if (m_sphereVBO) glDeleteBuffers(1, &m_sphereVBO);
    if (m_sphereEBO) glDeleteBuffers(1, &m_sphereEBO);

    // torus
    if (m_torusVAO) glDeleteVertexArrays(1, &m_torusVAO);
    if (m_torusVBO) glDeleteBuffers(1, &m_torusVBO);
    if (m_torusEBO) glDeleteBuffers(1, &m_torusEBO);

    // tapered cylinder
    if (m_taperVAO) glDeleteVertexArrays(1, &m_taperVAO);
    if (m_taperVBO) glDeleteBuffers(1, &m_taperVBO);
    if (m_taperEBO) glDeleteBuffers(1, &m_taperEBO);
}

void ShapeMeshes::LoadPlaneMesh()
{
    if (m_planeVAO != 0) return;

    std::vector<float> v;
    v.reserve(6 * 8);

    glm::vec3 n(0.0f, 1.0f, 0.0f);

    PushVertex(v, { -0.5f, 0.0f, -0.5f }, n, { 0.0f, 0.0f });
    PushVertex(v, { 0.5f, 0.0f, -0.5f }, n, { 1.0f, 0.0f });
    PushVertex(v, { 0.5f, 0.0f,  0.5f }, n, { 1.0f, 1.0f });

    PushVertex(v, { -0.5f, 0.0f, -0.5f }, n, { 0.0f, 0.0f });
    PushVertex(v, { 0.5f, 0.0f,  0.5f }, n, { 1.0f, 1.0f });
    PushVertex(v, { -0.5f, 0.0f,  0.5f }, n, { 0.0f, 1.0f });

    UploadNonIndexed(m_planeVAO, m_planeVBO, v);
}

void ShapeMeshes::DrawPlaneMesh()
{
    glBindVertexArray(m_planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void ShapeMeshes::LoadBoxMesh()
{
    if (m_boxVAO != 0) return;

    std::vector<float> v;
    v.reserve(36 * 8);

    // 6 faces, 2 triangles each, 3 verts each = 36 verts

    // +Y (top)
    {
        glm::vec3 n(0.0f, 1.0f, 0.0f);
        PushVertex(v, { -0.5f, 0.5f, -0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { 0.5f, 0.5f, -0.5f }, n, { 1.0f, 0.0f });
        PushVertex(v, { 0.5f, 0.5f,  0.5f }, n, { 1.0f, 1.0f });

        PushVertex(v, { -0.5f, 0.5f, -0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { 0.5f, 0.5f,  0.5f }, n, { 1.0f, 1.0f });
        PushVertex(v, { -0.5f, 0.5f,  0.5f }, n, { 0.0f, 1.0f });
    }

    // -Y (bottom)
    {
        glm::vec3 n(0.0f, -1.0f, 0.0f);
        PushVertex(v, { -0.5f, -0.5f,  0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { 0.5f, -0.5f,  0.5f }, n, { 1.0f, 0.0f });
        PushVertex(v, { 0.5f, -0.5f, -0.5f }, n, { 1.0f, 1.0f });

        PushVertex(v, { -0.5f, -0.5f,  0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { 0.5f, -0.5f, -0.5f }, n, { 1.0f, 1.0f });
        PushVertex(v, { -0.5f, -0.5f, -0.5f }, n, { 0.0f, 1.0f });
    }

    // +Z (front)
    {
        glm::vec3 n(0.0f, 0.0f, 1.0f);
        PushVertex(v, { -0.5f, -0.5f, 0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { 0.5f, -0.5f, 0.5f }, n, { 1.0f, 0.0f });
        PushVertex(v, { 0.5f,  0.5f, 0.5f }, n, { 1.0f, 1.0f });

        PushVertex(v, { -0.5f, -0.5f, 0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { 0.5f,  0.5f, 0.5f }, n, { 1.0f, 1.0f });
        PushVertex(v, { -0.5f,  0.5f, 0.5f }, n, { 0.0f, 1.0f });
    }

    // -Z (back)
    {
        glm::vec3 n(0.0f, 0.0f, -1.0f);
        PushVertex(v, { 0.5f, -0.5f, -0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { -0.5f, -0.5f, -0.5f }, n, { 1.0f, 0.0f });
        PushVertex(v, { -0.5f,  0.5f, -0.5f }, n, { 1.0f, 1.0f });

        PushVertex(v, { 0.5f, -0.5f, -0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { -0.5f,  0.5f, -0.5f }, n, { 1.0f, 1.0f });
        PushVertex(v, { 0.5f,  0.5f, -0.5f }, n, { 0.0f, 1.0f });
    }

    // +X (right)
    {
        glm::vec3 n(1.0f, 0.0f, 0.0f);
        PushVertex(v, { 0.5f, -0.5f,  0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { 0.5f, -0.5f, -0.5f }, n, { 1.0f, 0.0f });
        PushVertex(v, { 0.5f,  0.5f, -0.5f }, n, { 1.0f, 1.0f });

        PushVertex(v, { 0.5f, -0.5f,  0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { 0.5f,  0.5f, -0.5f }, n, { 1.0f, 1.0f });
        PushVertex(v, { 0.5f,  0.5f,  0.5f }, n, { 0.0f, 1.0f });
    }

    // -X (left)
    {
        glm::vec3 n(-1.0f, 0.0f, 0.0f);
        PushVertex(v, { -0.5f, -0.5f, -0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { -0.5f, -0.5f,  0.5f }, n, { 1.0f, 0.0f });
        PushVertex(v, { -0.5f,  0.5f,  0.5f }, n, { 1.0f, 1.0f });

        PushVertex(v, { -0.5f, -0.5f, -0.5f }, n, { 0.0f, 0.0f });
        PushVertex(v, { -0.5f,  0.5f,  0.5f }, n, { 1.0f, 1.0f });
        PushVertex(v, { -0.5f,  0.5f, -0.5f }, n, { 0.0f, 1.0f });
    }

    UploadNonIndexed(m_boxVAO, m_boxVBO, v);
}

void ShapeMeshes::DrawBoxMesh()
{
    glBindVertexArray(m_boxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void ShapeMeshes::LoadCylinderMesh()
{
    if (m_cylVAO != 0) return;

    const unsigned int slices = 48;

    std::vector<float> verts;
    std::vector<unsigned int> idx;

    // Build side vertices: (slices+1)*2
    for (unsigned int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float a = t * 2.0f * PI;

        float x = 0.5f * std::cos(a);
        float z = 0.5f * std::sin(a);

        glm::vec3 n(x, 0.0f, z);
        n = glm::normalize(n);

        // bottom ring
        PushVertex(verts, { x, -0.5f, z }, n, { t, 0.0f });
        // top ring
        PushVertex(verts, { x,  0.5f, z }, n, { t, 1.0f });
    }

    // Side indices
    for (unsigned int i = 0; i < slices; i++)
    {
        unsigned int b0 = i * 2;
        unsigned int t0 = b0 + 1;
        unsigned int b1 = (i + 1) * 2;
        unsigned int t1 = b1 + 1;

        idx.push_back(b0); idx.push_back(t0); idx.push_back(t1);
        idx.push_back(b0); idx.push_back(t1); idx.push_back(b1);
    }

    // Top cap center
    unsigned int topCenter = (unsigned int)(verts.size() / 8);
    PushVertex(verts, { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.5f, 0.5f });

    // Top cap ring
    unsigned int topStart = (unsigned int)(verts.size() / 8);
    for (unsigned int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float a = t * 2.0f * PI;

        float x = 0.5f * std::cos(a);
        float z = 0.5f * std::sin(a);

        PushVertex(verts, { x, 0.5f, z }, { 0.0f, 1.0f, 0.0f }, { (x + 0.5f), (z + 0.5f) });
    }

    for (unsigned int i = 0; i < slices; i++)
    {
        unsigned int v0 = topStart + i;
        unsigned int v1 = topStart + i + 1;
        idx.push_back(topCenter); idx.push_back(v0); idx.push_back(v1);
    }

    // Bottom cap center
    unsigned int botCenter = (unsigned int)(verts.size() / 8);
    PushVertex(verts, { 0.0f, -0.5f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.5f, 0.5f });

    // Bottom cap ring
    unsigned int botStart = (unsigned int)(verts.size() / 8);
    for (unsigned int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float a = t * 2.0f * PI;

        float x = 0.5f * std::cos(a);
        float z = 0.5f * std::sin(a);

        PushVertex(verts, { x, -0.5f, z }, { 0.0f, -1.0f, 0.0f }, { (x + 0.5f), (z + 0.5f) });
    }

    for (unsigned int i = 0; i < slices; i++)
    {
        unsigned int v0 = botStart + i;
        unsigned int v1 = botStart + i + 1;
        idx.push_back(botCenter); idx.push_back(v1); idx.push_back(v0);
    }

    UploadIndexed(m_cylVAO, m_cylVBO, m_cylEBO, verts, idx);
    m_cylIndexCount = (unsigned int)idx.size();
}

void ShapeMeshes::DrawCylinderMesh()
{
    glBindVertexArray(m_cylVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)m_cylIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void ShapeMeshes::LoadConeMesh()
{
    if (m_coneVAO != 0) return;

    const unsigned int slices = 48;

    std::vector<float> verts;
    std::vector<unsigned int> idx;

    // Apex
    unsigned int apex = 0;
    PushVertex(verts, { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.5f, 1.0f });

    // Base ring (slices+1)
    unsigned int baseStart = (unsigned int)(verts.size() / 8);
    for (unsigned int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float a = t * 2.0f * PI;

        float x = 0.5f * std::cos(a);
        float z = 0.5f * std::sin(a);

        // side normal approximation
        glm::vec3 n(x, 0.5f, z);
        n = glm::normalize(n);

        PushVertex(verts, { x, -0.5f, z }, n, { t, 0.0f });
    }

    // Side triangles
    for (unsigned int i = 0; i < slices; i++)
    {
        unsigned int v0 = baseStart + i;
        unsigned int v1 = baseStart + i + 1;
        idx.push_back(apex); idx.push_back(v0); idx.push_back(v1);
    }

    // Base center
    unsigned int baseCenter = (unsigned int)(verts.size() / 8);
    PushVertex(verts, { 0.0f, -0.5f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.5f, 0.5f });

    // Base ring for cap (separate normals)
    unsigned int capStart = (unsigned int)(verts.size() / 8);
    for (unsigned int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float a = t * 2.0f * PI;

        float x = 0.5f * std::cos(a);
        float z = 0.5f * std::sin(a);

        PushVertex(verts, { x, -0.5f, z }, { 0.0f, -1.0f, 0.0f }, { (x + 0.5f), (z + 0.5f) });
    }

    for (unsigned int i = 0; i < slices; i++)
    {
        unsigned int v0 = capStart + i;
        unsigned int v1 = capStart + i + 1;
        idx.push_back(baseCenter); idx.push_back(v1); idx.push_back(v0);
    }

    UploadIndexed(m_coneVAO, m_coneVBO, m_coneEBO, verts, idx);
    m_coneIndexCount = (unsigned int)idx.size();
}

void ShapeMeshes::DrawConeMesh()
{
    glBindVertexArray(m_coneVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)m_coneIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void ShapeMeshes::LoadSphereMesh()
{
    if (m_sphereVAO != 0) return;

    const unsigned int stacks = 24;
    const unsigned int slices = 48;

    std::vector<float> verts;
    std::vector<unsigned int> idx;

    for (unsigned int i = 0; i <= stacks; i++)
    {
        float v = (float)i / (float)stacks;
        float phi = v * PI;

        for (unsigned int j = 0; j <= slices; j++)
        {
            float u = (float)j / (float)slices;
            float theta = u * 2.0f * PI;

            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);

            glm::vec3 n(x, y, z);
            glm::vec3 p = 0.5f * n;

            PushVertex(verts, p, glm::normalize(n), { u, 1.0f - v });
        }
    }

    for (unsigned int i = 0; i < stacks; i++)
    {
        for (unsigned int j = 0; j < slices; j++)
        {
            unsigned int row1 = i * (slices + 1);
            unsigned int row2 = (i + 1) * (slices + 1);

            unsigned int a = row1 + j;
            unsigned int b = row1 + j + 1;
            unsigned int c = row2 + j;
            unsigned int d = row2 + j + 1;

            idx.push_back(a); idx.push_back(c); idx.push_back(d);
            idx.push_back(a); idx.push_back(d); idx.push_back(b);
        }
    }

    UploadIndexed(m_sphereVAO, m_sphereVBO, m_sphereEBO, verts, idx);
    m_sphereIndexCount = (unsigned int)idx.size();
}

void ShapeMeshes::DrawSphereMesh()
{
    glBindVertexArray(m_sphereVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)m_sphereIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void ShapeMeshes::LoadTorusMesh()
{
    if (m_torusVAO != 0) return;

    const unsigned int ringSeg = 48;
    const unsigned int tubeSeg = 24;
    const float R = 0.35f; // major radius
    const float r = 0.15f; // minor radius

    std::vector<float> verts;
    std::vector<unsigned int> idx;

    for (unsigned int i = 0; i <= ringSeg; i++)
    {
        float u = (float)i / (float)ringSeg;
        float a = u * 2.0f * PI;

        glm::vec3 ringCenter(R * std::cos(a), 0.0f, R * std::sin(a));

        for (unsigned int j = 0; j <= tubeSeg; j++)
        {
            float v = (float)j / (float)tubeSeg;
            float b = v * 2.0f * PI;

            glm::vec3 local(r * std::cos(b), r * std::sin(b), 0.0f);

            // rotate local around Y by angle a
            float lx = local.x * std::cos(a) + local.z * std::sin(a);
            float lz = -local.x * std::sin(a) + local.z * std::cos(a);

            glm::vec3 p = ringCenter + glm::vec3(lx, local.y, lz);
            glm::vec3 n = glm::normalize(glm::vec3(lx, local.y, lz));

            PushVertex(verts, p, n, { u, v });
        }
    }

    for (unsigned int i = 0; i < ringSeg; i++)
    {
        for (unsigned int j = 0; j < tubeSeg; j++)
        {
            unsigned int row1 = i * (tubeSeg + 1);
            unsigned int row2 = (i + 1) * (tubeSeg + 1);

            unsigned int a = row1 + j;
            unsigned int b = row1 + j + 1;
            unsigned int c = row2 + j;
            unsigned int d = row2 + j + 1;

            idx.push_back(a); idx.push_back(c); idx.push_back(d);
            idx.push_back(a); idx.push_back(d); idx.push_back(b);
        }
    }

    UploadIndexed(m_torusVAO, m_torusVBO, m_torusEBO, verts, idx);
    m_torusIndexCount = (unsigned int)idx.size();
}

void ShapeMeshes::DrawTorusMesh()
{
    glBindVertexArray(m_torusVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)m_torusIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void ShapeMeshes::LoadTaperedCylinderMesh()
{
    if (m_taperVAO != 0) return;

    const unsigned int slices = 48;
    const float r0 = 0.5f;  // bottom radius
    const float r1 = 0.25f; // top radius

    std::vector<float> verts;
    std::vector<unsigned int> idx;

    // side vertices (slices+1)*2
    for (unsigned int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float a = t * 2.0f * PI;

        float xb = r0 * std::cos(a);
        float zb = r0 * std::sin(a);

        float xt = r1 * std::cos(a);
        float zt = r1 * std::sin(a);

        // normal for frustum side
        glm::vec3 n(xb, (r0 - r1), zb);
        n = glm::normalize(n);

        PushVertex(verts, { xb, -0.5f, zb }, n, { t, 0.0f });
        PushVertex(verts, { xt,  0.5f, zt }, n, { t, 1.0f });
    }

    for (unsigned int i = 0; i < slices; i++)
    {
        unsigned int b0 = i * 2;
        unsigned int t0 = b0 + 1;
        unsigned int b1 = (i + 1) * 2;
        unsigned int t1 = b1 + 1;

        idx.push_back(b0); idx.push_back(t0); idx.push_back(t1);
        idx.push_back(b0); idx.push_back(t1); idx.push_back(b1);
    }

    UploadIndexed(m_taperVAO, m_taperVBO, m_taperEBO, verts, idx);
    m_taperIndexCount = (unsigned int)idx.size();
}

void ShapeMeshes::DrawTaperedCylinderMesh()
{
    glBindVertexArray(m_taperVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)m_taperIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}