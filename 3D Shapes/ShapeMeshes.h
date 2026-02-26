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
    // Plane
    GLuint m_planeVAO = 0;
    GLuint m_planeVBO = 0;

    // Box
    GLuint m_boxVAO = 0;
    GLuint m_boxVBO = 0;

    // Cylinder
    GLuint m_cylVAO = 0;
    GLuint m_cylVBO = 0;
    GLuint m_cylEBO = 0;
    unsigned int m_cylIndexCount = 0;

    // Cone
    GLuint m_coneVAO = 0;
    GLuint m_coneVBO = 0;
    GLuint m_coneEBO = 0;
    unsigned int m_coneIndexCount = 0;

    // Sphere
    GLuint m_sphereVAO = 0;
    GLuint m_sphereVBO = 0;
    GLuint m_sphereEBO = 0;
    unsigned int m_sphereIndexCount = 0;

    // Torus
    GLuint m_torusVAO = 0;
    GLuint m_torusVBO = 0;
    GLuint m_torusEBO = 0;
    unsigned int m_torusIndexCount = 0;

    // Tapered cylinder
    GLuint m_taperVAO = 0;
    GLuint m_taperVBO = 0;
    GLuint m_taperEBO = 0;
    unsigned int m_taperIndexCount = 0;
};