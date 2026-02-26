#pragma once
#include <GL/glew.h>

class ShapeMeshes
{
public:
    ShapeMeshes();
    ~ShapeMeshes();

    void LoadPlaneMesh();
    void DrawPlaneMesh();

    void LoadBoxMesh();
    void DrawBoxMesh();

    void LoadCylinderMesh();
    void DrawCylinderMesh();

    void LoadConeMesh();
    void DrawConeMesh();

    void LoadSphereMesh();
    void DrawSphereMesh();

    void LoadTorusMesh();
    void DrawTorusMesh();

    void LoadTaperedCylinderMesh();
    void DrawTaperedCylinderMesh();

private:
    GLuint m_planeVAO = 0, m_planeVBO = 0;
    GLuint m_boxVAO = 0, m_boxVBO = 0;
    GLuint m_cylVAO = 0, m_cylVBO = 0, m_cylEBO = 0;
    GLuint m_coneVAO = 0, m_coneVBO = 0, m_coneEBO = 0;
    GLuint m_sphereVAO = 0, m_sphereVBO = 0, m_sphereEBO = 0;
    GLuint m_torusVAO = 0, m_torusVBO = 0, m_torusEBO = 0;
    GLuint m_taperVAO = 0, m_taperVBO = 0, m_taperEBO = 0;

    unsigned int m_cylIndexCount = 0;
    unsigned int m_coneIndexCount = 0;
    unsigned int m_sphereIndexCount = 0;
    unsigned int m_torusIndexCount = 0;
    unsigned int m_taperIndexCount = 0;
};