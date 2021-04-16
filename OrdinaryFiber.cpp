#include <stdexcept>
#include <iostream>

#include "OrdinaryFiber.h"
#include "Pattern.h"
#include "Pattern2.h"

enum RENDER {
    TEST0,
    TEST1,
    TEST2,
    TEST3,
    TEST4,
    TEST5,
    TEST6,
    TEST7,
    TEST8,
    TEST9,
    TEST10,
    LIVE
};

OrdinaryFiber::OrdinaryFiber(const Fiber& fiber) : 
    SHADOW_WIDTH(1024),
    SHADOW_HEIGHT(1024),
    fbBound(false),
    depthTextureBound(false),
    FiberDrawable::FiberDrawable(fiber)
{}

OrdinaryFiber::~OrdinaryFiber()
{
	FiberDrawable::~FiberDrawable();
}

void OrdinaryFiber::create(std::vector<Fabric*>& fabrics)
{
    std::vector<Strand> strands;
    std::vector<ControlPoint> points;

    Pattern2 pattern = Pattern2(m_fiber);

    for (Fabric* fabric : fabrics)
    {
        FabricVertex* f = *fabric->begin();
        std::vector<Strand> strands = pattern.fabricTraversal(f, fabric->position);
        addStrands(strands);
    }

    // Store the control points in Vertex Buffer Object
    FiberDrawable::create();

    // Depth Map
    generateDepthMap();

    // Clear std::vectors no longer needed
    m_points.clear();
    m_indices.clear();
}

void OrdinaryFiber::create()
{
    std::cout << "Creating ordinary fiber drawable" << std::endl;
    // Create control points

    RENDER render = TEST8;
    std::vector<Strand> strands;
    std::vector<ControlPoint> points;

    switch (render)
    {
    case TEST0:
        // Basic yarn along the X-axis
        // STATUS: passed
        points.push_back(ControlPoint{ glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0, 0 });
        points.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1, 0.01f });
        points.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2, 0.99f });
        points.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3, 1.f });
        strands.push_back(Strand{ points });

        addStrands(strands);
        break;
    case TEST1:
        // Additional control point. Should render basic yarn from previous test.
        // STATUS: passed
        points.push_back(ControlPoint{ glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0, 0 });
        points.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1, 0.01f });
        points.push_back(ControlPoint{ glm::vec3(0.25f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2, 0.25f });
        points.push_back(ControlPoint{ glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3, 0.5f });
        points.push_back(ControlPoint{ glm::vec3(0.75f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 4, 0.75f });
        points.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 5, 0.99f });
        points.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 6, 1.f });
        strands.push_back(Strand{ points });
        addStrands(strands);
        break;
    case TEST2:
        // Additional strand on top of basic yarn.
        // STATUS: passed
        points.push_back(ControlPoint{ glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0, 0 });
        points.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1, 0.01f });
        points.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2, 0.99f });
        points.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3, 1.f });
        strands.push_back(Strand{ points });

        points.clear();
        points.push_back(ControlPoint{ glm::vec3(0.0f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 4, 0 });
        points.push_back(ControlPoint{ glm::vec3(0.01f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 5, 0.01f });
        points.push_back(ControlPoint{ glm::vec3(0.99f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 6, 0.99f });
        points.push_back(ControlPoint{ glm::vec3(1.f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 7, 1.f });
        strands.push_back(Strand{ points });

        addStrands(strands);
        break;
    case TEST3:
        // Basic yarn along the Y-axis
        // STATUS: passed
        points.push_back(ControlPoint{ glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 0, 0.f });
        points.push_back(ControlPoint{ glm::vec3(0.f, 0.01f, 0.f), glm::vec3(1.f, 0.f, 0.f), 1, 0.01f });
        points.push_back(ControlPoint{ glm::vec3(0.f, 0.99f, 0.f), glm::vec3(1.f, 0.f, 0.f), 3, 0.99f });
        points.push_back(ControlPoint{ glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 4, 1.f });
        strands.push_back(Strand{ points });
        addStrands(strands);
        break;
    case TEST4:
    {
        // Basic weave pattern - horizontal
        // TODO: make fiber_center = yarn_center in fiber_tess_eval.glsl
        // TODO: remove texture effect from fiber_fragment.glsl
        // TODO: make line height equal to yarn_radius * 2 in fiber_gemoetry.glsl
        // STATUS: passed
        Pattern pattern = Pattern(m_fiber);
        strands = pattern._getHorizontalStrand();
        addStrands(strands);
    }
    break;
    case TEST5:
    {
        // Basic weave pattern - vertical
        // TODO: make fiber_center = yarn_center in fiber_tess_eval.glsl
        // TODO: remove texture effect from fiber_fragment.glsl
        // TODO: make line height equal to yarn_radius * 2 in fiber_gemoetry.glsl
        // STATUS: passed
        Pattern pattern = Pattern(m_fiber);
        strands = pattern._getVerticalStrand();
        addStrands(strands);
    }
    break;
    case TEST6:
    {
        // Basic weave pattern
        // TODO: make fiber_center = yarn_center in fiber_tess_eval.glsl
        // TODO: remove texture effect from fiber_fragment.glsl
        // TODO: make line height equal to yarn_radius * 2 in fiber_gemoetry.glsl
        // STATUS: passed
        Pattern pattern = Pattern(m_fiber);
        strands = pattern.getBasicWeave(10);
        addStrands(strands);
    }
    break;
    case TEST7:
    {
        Pattern pattern = Pattern(m_fiber);
        strands = pattern.getBasicWeave(50);
        addStrands(strands);
    }
    break;
    case TEST8:
    {
        // Basic weave pattern
        // TODO: make fiber_center = yarn_center in fiber_tess_eval.glsl
        // TODO: remove texture effect from fiber_fragment.glsl
        // TODO: make line height equal to yarn_radius * 2 in fiber_gemoetry.glsl
        // STATUS: passed
        Pattern pattern = Pattern(m_fiber);
        strands = pattern.getBasicWeave(10);
        addStrands(strands);

       // pattern.updatePosition(glm::vec3(-0.2, -1, -1));
        //strands = pattern.getBasicWeave(10);
        //addStrands(strands);
    }
    break;
    case TEST9:
    {
        float length = 10 * 4 * m_fiber.yarn_radius;
        float horizontalStretch = 1.f; // modify to test
        float verticalStretch = 1.f; // modify to test
        float zOffsetP1 = 0;// -0.1f; // modify to test
        float zOffsetP2 = 0;// 0.5f; // modify to test
        float zOffsetP3 = 0;// 0.2; // modify to test
        float zOffsetP4 = 0;// -0.5f; // modify to test
        Pattern2 pattern = Pattern2(m_fiber); 
        std::vector<glm::vec3> points;
        points.push_back(glm::vec3(0, 0, zOffsetP1));
        points.push_back(glm::vec3(length * horizontalStretch, 0, zOffsetP2));
        points.push_back(glm::vec3(length * horizontalStretch, length * verticalStretch, zOffsetP3));
        points.push_back(glm::vec3(0, length * verticalStretch, zOffsetP4));
        strands = pattern.getUnitPattern(points, length); // 1 cm per edge
        addStrands(strands);
    }
    break;
    case TEST10:
    {
        Pattern2 pattern = Pattern2(m_fiber);
        float length = round(10 * 4 * m_fiber.yarn_radius);
        Fabric *fabric1 = new SquareFabric(
            "square", glm::vec3(0.f, 0.f, 0.f), glm::vec2(2, 2), glm::vec2(1, 1), .5f);
        Fabric* fabric2 = new SquareFabric(
            "square", glm::vec3(0.f, 1.f, 0.f), glm::vec2(2, 2), glm::vec2(1, 1), .5f);
        FabricVertex* f1 = *fabric1->begin();
        FabricVertex* f2 = *fabric2->begin();
        std::vector<Strand> strands1 = pattern.fabricTraversal(f1, fabric1->position);
        std::vector<Strand> strands2 = pattern.fabricTraversal(f2, fabric2->position);

        addStrands(strands1);
        addStrands(strands2);
    }
    break;
    }



    // Store the control points in Vertex Buffer Object
    FiberDrawable::create();

    // Depth Map
    generateDepthMap();

    // Clear std::vectors no longer needed
    m_points.clear();
    m_indices.clear();
}

void OrdinaryFiber::generateDepthMap()
{
    // Create framebuffer object for rendering the depth map
    glGenFramebuffers(1, &depthMapFBO);

    // create 2D texture for framebuffer's depth buffer
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    depthTextureBound = true;

    // attach generated depth texture to framebuffer's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); // No color buffer needed
    glReadBuffer(GL_NONE); // No color buffer needed
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fbBound = true;
}

void OrdinaryFiber::setHeightTexture(GLuint i)
{
	heightTexBound = true;
	heightTexture = i;
}

void OrdinaryFiber::setNormalTexture(GLuint i)
{
	normalTexBound = true;
	normalTexture = i;
}

void OrdinaryFiber::setAlphaTexture(GLuint i)
{
	alphaTexBound = true;
	alphaTexture = i;
}

bool OrdinaryFiber::bindDepthMap()
{
    if (fbBound && depthTextureBound)
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    return fbBound && depthTextureBound;
}

void OrdinaryFiber::bindHeightTexture()
{
	if (heightTexBound)
		glBindTexture(GL_TEXTURE_2D, heightTexture);
	else
		throw std::runtime_error("No height texture to bind to.");
}
void OrdinaryFiber::bindNormalTexture()
{
	if (normalTexBound)
		glBindTexture(GL_TEXTURE_2D, normalTexture);
	else
		throw std::runtime_error("No normal texture to bind to.");
}
void OrdinaryFiber::bindAlphaTexture()
{
	if (alphaTexBound)
		glBindTexture(GL_TEXTURE_2D, alphaTexture);
	else
		throw std::runtime_error("No alpha texture to bind to.");
}

void OrdinaryFiber::bindDepthTexture()
{
    if (depthTextureBound)
        glBindTexture(GL_TEXTURE_2D, depthMap);
    else
        throw std::runtime_error("No depth texture to bind to.");
}