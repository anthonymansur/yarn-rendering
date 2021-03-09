#include "Fabric.h"
#include <iostream>
#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_cross_product.hpp>
#include <list>
#include <array>
#include <math.h>


void Fabric::mult(glm::mat3 m)
{
    for (auto& v : *this)
    {
        v->localPos = m * v->localPos;
    }
    //position = m * position;
}

FabricVertex*& iterator::operator*() const
{
    return this->fab->next(this->idx);
}



FabricVertex::FabricVertex(glm::vec3 p, glm::vec2 l)
    : debugColor(glm::vec3(1.f, 1.f, 1.f)), origPos(p), localPos(p), tmpPos(p), edgeLength(l), negWarp(nullptr), posWarp(nullptr), negWeft(nullptr), posWeft(nullptr), other(nullptr), id(0), velocity(glm::vec3(0, 0, 0))
{
    attr.push_back(glm::vec3(0, 0, 0));
    attr.push_back(glm::vec3(0, 0, 0));
}

void FabricVertex::_reset()
{
    this->localPos = this->origPos;
    this->tmpPos = this->origPos;
    this->velocity = glm::vec3(0, 0, 0);
    this->attr.clear();
    attr.push_back(glm::vec3(0, 0, 0));
    attr.push_back(glm::vec3(0, 0, 0));
}


Fabric::Fabric(std::string name, glm::vec3 pos, float mass) : name(name), position(pos), mass(mass) {}

/*
Edge
*/
Edge::Edge()
{

}
Edge::~Edge()
{
    // DO NOT DELETE FabricVertex* -- owned and deleted by Fabric
    edgeVertices.clear();
}
// Calc angle and null direction
void Edge::calcProperties()
{
    std::array<int, 4> dirs = { 0 /*negWeft*/, 0 /*posWeft*/, 0 /*negWarp*/, 0 /*posWarp*/ };
    for (FabricVertex* v : edgeVertices)
    {
        // "Vote" for null directions
        if (v->negWeft == nullptr)
            dirs[0]++;
        if (v->posWeft == nullptr)
            dirs[1]++;
        if (v->negWarp == nullptr)
            dirs[2]++;
        if (v->posWarp == nullptr)
            dirs[3]++;
    }
    // Find which direction got "voted" on most
    int dir = 0;
    if (dirs[1] > dirs[0])
        dir = 1;
    if (dirs[2] > dir)
        dir = 2;
    if (dirs[3] > dir)
        dir = 3;
    this->nullDirection = dir;
    // Find reference angle
    glm::vec3 edgeDir = glm::normalize(edgeVertices.back()->localPos - edgeVertices.front()->localPos);
    glm::vec3 ref(0, 0, 1);
    float crossSin = glm::length(glm::cross(ref, edgeDir));
    float dotCos = glm::dot(ref, edgeDir);

}

/*

SQUARE FABRIC

*/
void SquareFabric::connect(Fabric* fab, int edgeA, int edgeB) { //TODO crashes when connecting twice fix
    // Edges are specified in counterclockwise order. 
    std::vector<FabricVertex*>& thisEdge = this->edges[edgeA];
    std::vector<FabricVertex*>& thatEdge = fab->edges[edgeB];

    for (float i = 0; i < thisEdge.size(); i++) {
        float percent = i / (float)thisEdge.size();
        thisEdge[(int)i]->debugColor = glm::vec3(percent, 0, 0);
    }
    for (float i = 0; i < thatEdge.size(); i++) {
        float percent = i / (float)thatEdge.size();
        thatEdge[(int)i]->debugColor = glm::vec3(percent, 0, 0);
    }

    // 1. Calculate relative angle
    glm::vec3 thisEdgeDir = glm::normalize(thisEdge.back()->localPos - thisEdge.front()->localPos);
    glm::vec3 thatEdgeDir = glm::normalize(thatEdge.back()->localPos - thatEdge.front()->localPos);

    // 2. Move 2nd Fabric to match placement of first one (rotation too)
    // 3. Get edge arrays (If multi-edge, just merge into one)
    // 4. Get loop skip ratio
    // 5. Connect through "other"






    // 1. Find rotation matrix needed to bring thatEdge onto -thisEdge
    glm::vec3 cross = glm::cross(thatEdgeDir, thisEdgeDir);
    float crossSin = glm::length(cross);
    float dotCos = glm::dot(thisEdgeDir, thatEdgeDir);
    glm::mat3 skew = glm::matrixCross3(cross);
    glm::mat3 R = glm::mat3(1) + skew + skew * skew * ((1 + dotCos) / (crossSin * crossSin));
    // 2. Rotate all vertices in second fabric by given amount
    //fab->mult(R);
    // 3. Translate 2nd fabric to align position with first
    //glm::vec3 offset = fab->position + thatEdge.front()->localPos - position - thisEdge.back()->localPos;
    //fab->position -= offset;
    // 4. Proper edge arrays


    /*float amountA = thisEdge.size(), amountB = thatEdge.size();
    st
    float ratio = glm::max(amountA, amountB) / glm::min(amountA, amountB);*/


}

SquareFabric::SquareFabric(std::string name, glm::vec3 pos, glm::vec3 pos2, int stringDensity, float mass) : Fabric(name, pos, mass), isString(true)
{
    float dim = glm::length(pos2 - pos);
    amount.x = dim * stringDensity + 1;
    amount.y = 1;
    float length = dim / (amount.x - 1);
    glm::vec3 dir = glm::normalize(pos2 - pos);
    edges.push_back(std::vector<FabricVertex*>(amount.x));
    for (int i = 0; i < amount.x; i++)
    {
        FabricVertex* v = new FabricVertex(dir * length * (float)i, glm::vec2(length, length));
        v->id = i;
        vertices.push_back(v);
        edges[0].push_back(v);

        // Assign pointers to preceding vertices
        FabricVertex* nWarp = get(glm::vec2(i - 1, 0));
        if (nWarp != nullptr)
        {
            v->negWarp = nWarp;
            nWarp->posWarp = v;
        }
    }

}

SquareFabric::SquareFabric(std::string name, glm::vec3 pos, glm::vec2 dim, glm::vec2 denFace, float mass) : Fabric(name, pos, mass), isString(false)
{
    // If chain, use chain constructor
    if (dim.y == 0 || dim.x == 0) {
        std::cerr << "Wrong fabric constructor used." << std::endl;
        return;
    }

    // Calculate how many vertices I need in each direction
    amount = dim * denFace + glm::vec2(1, 1); // Dimensions are taking as number of units, density as vertices per unit
    glm::vec2 length = dim / (amount - glm::vec2(1, 1)); // Or 1/den
    //std::cout << amount.x << " " << amount.y << " l: " << length.x << " " << length.y << std::endl;
    // Create the vertices
    for (int a = 0; a < 4; a++) edges.push_back(std::vector<FabricVertex*>());
    for (int j = 0; j < amount.y; j++)
    {
        for (int i = 0; i < amount.x; i++)
        {
            FabricVertex* curr = new FabricVertex(glm::vec3(i * length.x, 0, j * length.y), length);
            curr->id = j * amount.x + i;
            vertices.push_back(curr);
            if (j == 0) {
                edges[0].push_back(curr);
                //Don't forget to reverse to make it proper ccwise
            }
            if (j == amount.y - 1) {
                edges[2].push_back(curr);
            }

            // Assign pointers to preceding vertices
            FabricVertex* nWarp = get(glm::vec2(i - 1, j));
            FabricVertex* nWeft = get(glm::vec2(i, j - 1));
            if (nWarp != nullptr)
            {
                curr->negWarp = nWarp;
                nWarp->posWarp = curr;
            }
            if (nWeft != nullptr)
            {
                curr->negWeft = nWeft;
                nWeft->posWeft = curr;
            }
        }
    }
    // Add edges 2 and 4
    FabricVertex* curr = vertices.at(0);
    edges[1].push_back(curr);
    while (curr->posWeft != nullptr) {
        curr = curr->posWeft;
        edges[1].push_back(curr);
    }
    curr = get(glm::vec2(amount.x - 1, amount.y - 1));
    edges[3].push_back(curr);
    while (curr->negWeft != nullptr) {
        curr = curr->negWeft;
        edges[3].push_back(curr);
    }//TODO: do-while loop instead
    // Reverse edge 3 for ccwise
    std::reverse(edges[2].begin(), edges[2].end());

    for (std::vector<FabricVertex*>& edge : this->edges) {
        for (float i = 0; i < edge.size(); i++) {
            float percent = i / (float)edge.size();
            edge[(int)i]->debugColor = glm::vec3(0, percent, 0);
        }
    }
}

SquareFabric::~SquareFabric()
{
    Fabric::~Fabric();
    for (auto v : *this)
    {
        v->_reset();
        delete v;
        v = nullptr;
    }
    vertices.clear();
}

FabricVertex* SquareFabric::get(glm::vec2 rp)
{
    if (rp.x < 0 || rp.y < 0) {
        return nullptr;
    }
    int vectorIndex = rp.y * amount.x + rp.x;
    if (vertices.size() > vectorIndex) {
        return vertices.at(vectorIndex);
    }
    return nullptr;
}

void SquareFabric::create()
{
    std::vector<glm::vec4> faceVertices;
    std::vector<glm::vec4> faceNormals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> cols;
    std::vector<GLuint> idx;

    if (isString)
    {
        for (int i = 0; i < amount.x - 1; i++)
        {
            FabricVertex* v1 = get(glm::vec2(i, 0));
            FabricVertex* v2 = get(glm::vec2(i + 1, 0));
            //std::cout << v1->localPos.x << "," << v1->localPos.y << "," << v1->localPos.z << " - " << v2->localPos.x << "," << v2->localPos.y << "," << v2->localPos.z << std::endl;
            glm::vec3 dir = glm::normalize(v2->localPos - v1->localPos);
            glm::vec3 ncl = glm::normalize(glm::vec3((dir.x - 1, dir.y - 2, dir.z + 3)));
            glm::vec3 perp = glm::cross(dir, ncl);
            //Vertices
            faceVertices.push_back(glm::vec4(v1->localPos + this->position, 1));
            faceVertices.push_back(glm::vec4(v2->localPos + this->position, 1));
            faceVertices.push_back(glm::vec4(v1->localPos + this->position + perp * .01f, 1));
            faceVertices.push_back(glm::vec4(v2->localPos + this->position + perp * .01f, 1));
            //Idx
            idx.push_back(i * 4);
            idx.push_back(i * 4 + 1);
            idx.push_back(i * 4 + 2);
            idx.push_back(i * 4);
            idx.push_back(i * 4 + 2);
            idx.push_back(i * 4 + 3);
            //UVs (Should really use color here...)
            float uvl = 1.f / (amount.x - 1);
            /*uvs.push_back(glm::vec2(uvl * i, 0));
            uvs.push_back(glm::vec2(uvl * (i + 1), 1));*/
            uvs.push_back(glm::vec2(0, 0));
            uvs.push_back(glm::vec2(1, 0));
            uvs.push_back(glm::vec2(1, 1));
            uvs.push_back(glm::vec2(0, 1));
            //Fake normals
            glm::vec3 nor = glm::cross(dir, perp);
            faceNormals.push_back(glm::vec4(nor, 0));
            faceNormals.push_back(glm::vec4(nor, 0));
            faceNormals.push_back(glm::vec4(nor, 0));
            faceNormals.push_back(glm::vec4(nor, 0));
        }
    }
    else
    {
        for (int i = 0; i < amount.x - 1; i++)
        {
            for (int j = 0; j < amount.y - 1; j++)
            {
                //The vertex at (i, j) is the top left of each respective face.
                //We now want to add the 4 pos and normals of that face
                // As well as UV? and ix

                //How many faces have already been pushed?
                int curr = i * (amount.y - 1) + j;

                // Query 4 relevant vertices
                FabricVertex* v1 = get(glm::vec2(i, j));
                FabricVertex* v2 = get(glm::vec2(i + 1, j));
                FabricVertex* v3 = get(glm::vec2(i + 1, j + 1));
                FabricVertex* v4 = get(glm::vec2(i, j + 1));

                // Push pos MIGHT HAVE TO CHANGE ORDER
                faceVertices.push_back(glm::vec4(v4->localPos + this->position, 1));
                faceVertices.push_back(glm::vec4(v3->localPos + this->position, 1));
                faceVertices.push_back(glm::vec4(v2->localPos + this->position, 1));
                faceVertices.push_back(glm::vec4(v1->localPos + this->position, 1));
                // Calculate normal
                glm::vec3 ca = v2->localPos - v1->localPos;
                glm::vec3 cb = v4->localPos - v1->localPos;
                glm::vec3 nor = glm::cross(cb, ca); //Might be other way around
                // Push normals
                faceNormals.push_back(glm::vec4(nor, 0));
                faceNormals.push_back(glm::vec4(nor, 0));
                faceNormals.push_back(glm::vec4(nor, 0));
                faceNormals.push_back(glm::vec4(nor, 0));
                // Push Idx (counterclockwise!)
                idx.push_back(curr * 4);
                idx.push_back(curr * 4 + 1);
                idx.push_back(curr * 4 + 2);
                idx.push_back(curr * 4);
                idx.push_back(curr * 4 + 2);
                idx.push_back(curr * 4 + 3);
                // Calculate UV length (assuming single texture)
                glm::vec2 subUV = glm::vec2(1.0, 1.0) / (amount - glm::vec2(1, 1));
                // Push UVs
                glm::vec2 uv4((i + 1) * subUV.x, 1 - j * subUV.y);
                glm::vec2 uv3((i + 1) * subUV.x, 1 - (j + 1) * subUV.y);
                glm::vec2 uv2(i * subUV.x, 1 - (j + 1) * subUV.y);
                glm::vec2 uv1(i * subUV.x, 1 - j * subUV.y);

                glm::vec2 a(0, 0);
                glm::vec2 b(0, 1);
                glm::vec2 c(1, 0);
                glm::vec2 d(1, 1);

                uvs.push_back(uv2);
                uvs.push_back(uv3);
                uvs.push_back(uv4);
                uvs.push_back(uv1);

                if (this->debug) {
                    float a = (float)i * (float)j;
                    /*cols.push_back(glm::vec3(a, a, a));
                    cols.push_back(glm::vec3(a, a, a));
                    cols.push_back(glm::vec3(a, a, a));
                    cols.push_back(glm::vec3(a, a, a));*/
                    cols.push_back(v4->debugColor);
                    cols.push_back(v3->debugColor);
                    cols.push_back(v2->debugColor);
                    cols.push_back(v1->debugColor);
                }
            }
        }
    }
    count = idx.size();
    generateIdx();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    glBufferData(GL_ARRAY_BUFFER, faceVertices.size() * sizeof(glm::vec4), faceVertices.data(), GL_STATIC_DRAW);

    generateNor();
    glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    glBufferData(GL_ARRAY_BUFFER, faceNormals.size() * sizeof(glm::vec4), faceNormals.data(), GL_STATIC_DRAW);

    generateUV();
    glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);

    if (this->debug) {
        generateCol();
        glBindBuffer(GL_ARRAY_BUFFER, bufCol);
        glBufferData(GL_ARRAY_BUFFER, cols.size() * sizeof(glm::vec3), cols.data(), GL_STATIC_DRAW);
    }
}

void SquareFabric::reset(glm::vec3 pos)
{
    this->position = pos;
    for (auto& v : vertices)
    {
        v->_reset();
    }
}

std::size_t SquareFabric::size() const
{
    return amount.x * amount.y;
}

FabricVertex*& SquareFabric::next(std::size_t idx)
{
    return vertices[idx];
}


/*

POLY FABRIC

*/
void PolyFabric::connect(Fabric* fab, int edgeA, int edgeB) {}

/*
PolyFabric::PolyFabric(std::string name, glm::vec3 pos, std::vector<glm::vec2>& vertices, glm::vec2 denFace, float m) : Fabric(name, pos, m), amount(0)
{
    //TODO vertices Format: vertical segments should only be vertical thru 2 vertices.

    // Only allow polygons
    if (vertices.size() < 3) {
        //ERROR TODO
        return;
    }

    // Copy of vertices for later operations
    std::vector<glm::vec2> orig(vertices);

    //amount = dim * denFace + glm::vec2(1, 1); // Dimensions are taking as number of units, density as vertices per unit
    glm::vec2 length = 1.f / denFace;

    //Sort vertices by increasing x coordinate
    std::sort(vertices.begin(), vertices.end(), [](glm::vec2 a, glm::vec2 b) {return a.x < b.x; });

    // First and last
    const int start = 0, end = vertices.size() - 1;

    // Find where in orig the first vertex is
    int startOrigIdx = std::distance(orig.begin(), find(orig.begin(), orig.end(), vertices[start]));

    // Shift orig by that so that vertices are still ordered ccwise but start with least x one
    std::rotate(orig.begin(), orig.begin() + startOrigIdx, orig.end());

    // Find top and bot last vertices
    int endTopOrigIdx = -1, endBotOrigIdx = -1;
    {
        // Check if end segment is completely vertical
        bool verticalEnd = vertices[end].x == vertices[end - 1].x;
        // Greatest and second greatest x vertex
        int endOrigIdx1 = std::distance(orig.begin(), find(orig.begin(), orig.end(), vertices[end]));
        int endOrigIdx2 = std::distance(orig.begin(), find(orig.begin(), orig.end(), vertices[end - 1]));
        if (!verticalEnd)
        {
            // If both paths end in the same vertex, let them be the position in orig where the greatest-x vertex is
            endTopOrigIdx = endOrigIdx1;
            endBotOrigIdx = endOrigIdx1;
        }
        else
        {
            // If paths end in different vertex, let the top path end in the 'latter' vertex and bot path in 'earlier' one due to ccwise orientation
            endTopOrigIdx = endOrigIdx1 > endOrigIdx2 ? endOrigIdx1 : endOrigIdx2;
            endBotOrigIdx = endOrigIdx1 > endOrigIdx2 ? endOrigIdx2 : endOrigIdx1;
        }
    }

    // Create paths
    std::list<int> topPath, botPath;
    for (int idx = 0; idx <= endBotOrigIdx; idx++)
    {
        botPath.push_back(idx);
    }
    for (int idx = endTopOrigIdx; idx < orig.size(); idx++)
    {
        topPath.push_back(idx);
    }
    std::reverse(topPath.begin(), topPath.end());
    topPath.push_front(0);

    // Find min and max ys
    float minY = vertices[0].y, maxY = vertices[0].y;
    for (glm::vec2 v : vertices)
    {
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
    }

    // Save the first upper and lower segment
    int prevTopIdx = topPath.front(), prevBotIdx = botPath.front(); // Both parts start with 0 ad end with last one
    topPath.pop_front();
    botPath.pop_front();
    int currTopIdx = topPath.front(), currBotIdx = botPath.front();
    topPath.pop_front();
    botPath.pop_front();

    //2D Array to help with connecting vertices
    xVals = (vertices[end].x - vertices[start].x) / length.x;
    yVals = (maxY - minY) / length.y;
    arr = new FabricVertex **[xVals];
    for (int i = 0; i < xVals; i++)
    {
        arr[i] = new FabricVertex *[yVals];
        for (int j = 0; j < yVals; j++)
        {
            arr[i][j] = nullptr;
        }
    }
    //std::cout << "x0: " << vertices[start].x << ", x-1: " << vertices[end].x << std::endl;
    for (float x = vertices[start].x; x < vertices[end].x; x += length.x)
    {
        // TODO: x could exceed last x on the final segment, thus attempt to pop non present element. TODO TODO TODO
        // Make sure currently considered segments are up to date
        if (x > orig[currTopIdx].x)
        {
            prevTopIdx = currTopIdx;
            currTopIdx = topPath.front();
            topPath.pop_front();
        }
        if (x > orig[currBotIdx].x)
        {
            prevBotIdx = currBotIdx;
            currBotIdx = botPath.front();
            botPath.pop_front();
        }
        // Have all necessary elements ready
        glm::vec2   prevTop = orig[prevTopIdx],
            currTop = orig[currTopIdx],
            prevBot = orig[prevBotIdx],
            currBot = orig[currBotIdx];
        // Find segment slope
        float topSlope = (currTop.y - prevTop.y) / (currTop.x - prevTop.x);
        float botSlope = (currBot.y - prevBot.y) / (currBot.x - prevBot.x);
        // Find top and bot y coordinates for current x coordinate //TODO Vertical slope??
        float topY = prevTop.y + topSlope * (x - prevTop.x);
        float botY = prevBot.y + botSlope * (x - prevBot.x);
        //
        //std::cout << "y0: " << botY << ", y-1: " << topY << ", yl: " << length.y << std::endl;
        for (float y = botY; y < topY; y += length.y)
        {
            // Create vertex and connect....
            // Vertices have to be fit into a grid based on length
            // Perhaps approximate by rounding: First check how many in each segment, then round down starting spot based on length and connect from there
            glm::ivec2 idxCoords((x - vertices[start].x) / length.x, (y - minY) / length.y);
            FabricVertex* curr = new FabricVertex(glm::vec3(x, 0, y), length);
            curr->id = x + y; //TODO Remove this
            this->vertices.push_back(curr);
            std::cout << "(" << xVals << ", " << yVals << "): " << idxCoords.x << ", " << idxCoords.y << std::endl;
            arr[idxCoords.x][idxCoords.y] = curr;
            amount++;
        }
    }
    std::cout << amount << std::endl;
    // Connect them all
    for (int i = 0; i < xVals; i++)
    {
        for (int j = 0; j < yVals; j++)
        {
            FabricVertex* curr = arr[i][j];
            if (curr == nullptr) continue;
            FabricVertex* n = nullptr;
            //Four neighbors:
            if (i - 1 >= 0) {
                n = arr[i - 1][j];
                if (n != nullptr) {
                    curr->negWarp = n;
                    n->posWarp = curr;
                }
            }
            if (i + 1 < xVals) {
                n = arr[i + 1][j];
                if (n != nullptr) {
                    curr->posWarp = n;
                    n->negWarp = curr;
                }
            }
            if (j - 1 >= 0) {
                n = arr[i][j - 1];
                if (n != nullptr) {
                    curr->negWeft = n;
                    n->posWeft = curr;
                }
            }
            if (j + 1 < yVals) {
                n = arr[i][j + 1];
                if (n != nullptr) {
                    curr->posWeft = n;
                    n->negWeft = curr;
                }
            }
        }
    }

    // Clean up array
    for (int i = 0; i < xVals; i++)
    {
        delete[] arr[i];
    }
    delete[] arr;
}
*/

PolyFabric::PolyFabric(std::string name, glm::vec3 pos, std::vector<glm::vec2>& vertices, glm::vec2 denFace, float m) : Fabric(name, pos, m), amount(0)
{
    //TODO vertices Format: vertical segments should only be vertical thru 2 vertices.

    // Only allow polygons
    if (vertices.size() < 3) {
        //ERROR TODO
        return;
    }

    // Copy of vertices for later operations
    std::vector<glm::vec2> orig(vertices);

    //amount = dim * denFace + glm::vec2(1, 1); // Dimensions are taking as number of units, density as vertices per unit
    glm::vec2 length = 1.f / denFace;

    //Sort vertices by increasing x coordinate
    std::sort(vertices.begin(), vertices.end(), [](glm::vec2 a, glm::vec2 b) {return a.x < b.x; });

    // First and last
    const int start = 0, end = vertices.size() - 1;

    // Find where in orig the first vertex is
    int startOrigIdx = std::distance(orig.begin(), find(orig.begin(), orig.end(), vertices[start]));

    // Shift orig by that so that vertices are still ordered ccwise but start with least x one
    std::rotate(orig.begin(), orig.begin() + startOrigIdx, orig.end());

    // Find top and bot last vertices
    int endTopOrigIdx = -1, endBotOrigIdx = -1;
    {
        // Check if end segment is completely vertical
        bool verticalEnd = vertices[end].x == vertices[end - 1].x;
        // Greatest and second greatest x vertex
        int endOrigIdx1 = std::distance(orig.begin(), find(orig.begin(), orig.end(), vertices[end]));
        int endOrigIdx2 = std::distance(orig.begin(), find(orig.begin(), orig.end(), vertices[end - 1]));
        if (!verticalEnd)
        {
            // If both paths end in the same vertex, let them be the position in orig where the greatest-x vertex is
            endTopOrigIdx = endOrigIdx1;
            endBotOrigIdx = endOrigIdx1;
        }
        else
        {
            // If paths end in different vertex, let the top path end in the 'latter' vertex and bot path in 'earlier' one due to ccwise orientation
            endTopOrigIdx = endOrigIdx1 > endOrigIdx2 ? endOrigIdx1 : endOrigIdx2;
            endBotOrigIdx = endOrigIdx1 > endOrigIdx2 ? endOrigIdx2 : endOrigIdx1;
        }
    }

    // Create paths
    std::list<int> topPath, botPath;
    for (int idx = 0; idx <= endBotOrigIdx; idx++)
    {
        botPath.push_back(idx);
    }
    for (int idx = endTopOrigIdx; idx < orig.size(); idx++)
    {
        topPath.push_back(idx);
    }
    std::reverse(topPath.begin(), topPath.end());
    topPath.push_front(0);

    // Find min and max ys
    float minY = vertices[0].y, maxY = vertices[0].y;
    for (glm::vec2 v : vertices)
    {
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
    }

    // Save the first upper and lower segment
    int prevTopIdx = topPath.front(), prevBotIdx = botPath.front(); // Both parts start with 0 ad end with last one
    topPath.pop_front();
    botPath.pop_front();
    int currTopIdx = topPath.front(), currBotIdx = botPath.front();
    topPath.pop_front();
    botPath.pop_front();

    //2D Array to help with connecting vertices
    xVals = ceil((vertices[end].x - vertices[start].x) / length.x);
    yVals = ceil((maxY - minY) / length.y);
    //arr = new FabricVertex * *[xVals];
    for (int i = 0; i < xVals; i++)
    {
        tmpArr.push_back(std::vector<FabricVertex*>());
        for (int j = 0; j < yVals; j++)
        {
            tmpArr.at(i).push_back(nullptr);
        }
    }
    std::cout << "Size: " << xVals << ", " << yVals << "Mins: " << vertices[start].x << ", " << minY << "Length: " << length.x << ", " << length.y << std::endl;

    ////float lX = vertices[start].x;
    //float rXo = vertices[end].x;
    //// Calculate how much the top/bot estimates are off from fitting the intervals perfectly
    ////float rmL = abs(fmod(lX, length.x));//
    //float rmR = abs(fmod(rXo - vertices[start].x, length.x));
    //// If deviation is less than half, remove deviation. If more than half, add (length-deviation) to widen grid.
    ////lX += rmL <= length.x / 2 ? -rmT : length.y - rmT;
    //printf("rX 1: %f, rmR: %f\n", rXo, rmR);
    //float rX = rXo + ((rmR <= length.x / 2) ? -rmR : length.x - rmR);
    //printf("rX 1: %f, rX 2: %f\n", rXo, rX);
    //std::cout << "DIVIDE REMAIN: " << (rX - vertices[start].x) / length.x << " " << fmod((rX - vertices[start].x), length.x) << std::endl;

    float currX = 0;
    for (float xi = 0; xi < xVals; xi++) {
        //}
        //for (float x = vertices[start].x; x < rX; x += length.x)
        //{
            // TODO: x could exceed last x on the final segment, thus attempt to pop non present element. TODO TODO TODO
            // Make sure currently considered segments are up to date
        if (currX > orig[currTopIdx].x)
        {
            prevTopIdx = currTopIdx;
            currTopIdx = topPath.front();
            topPath.pop_front();
        }
        if (currX > orig[currBotIdx].x)
        {
            prevBotIdx = currBotIdx;
            currBotIdx = botPath.front();
            botPath.pop_front();
        }
        // Have all necessary elements ready
        glm::vec2   prevTop = orig[prevTopIdx],
            currTop = orig[currTopIdx],
            prevBot = orig[prevBotIdx],
            currBot = orig[currBotIdx];
        // Find segment slope
        float topSlope = (currTop.y - prevTop.y) / (currTop.x - prevTop.x);
        float botSlope = (currBot.y - prevBot.y) / (currBot.x - prevBot.x);
        // Find top and bot y coordinates for current x coordinate //TODO Vertical slope??
        float topY = prevTop.y + topSlope * (currX - prevTop.x);
        float botY = prevBot.y + botSlope * (currX - prevBot.x);

        // Calculate how much the top/bot estimates are off from fitting the intervals perfectly
        float rmT = abs(fmod(topY, length.y));//
        float rmB = abs(fmod(botY, length.y));
        // If deviation is less than half, remove deviation. If more than half, add (length-deviation) to widen grid.
        topY += rmT <= length.y / 2 ? -rmT : length.y - rmT;
        botY += rmB <= length.y / 2 ? rmB : rmB - length.y;


        //
        //std::cout << "y0: " << botY << ", y-1: " << topY << ", yl: " << length.y << std::endl;
        for (float y = botY; y < topY; y += length.y)
        {
            // Create vertex and connect....
            // Vertices have to be fit into a grid based on length
            // Perhaps approximate by rounding: First check how many in each segment, then round down starting spot based on length and connect from there
            glm::ivec2 idxCoords(xi, (y - minY) / length.y);
            if (idxCoords.x == xVals || idxCoords.y == yVals) continue;
            FabricVertex* curr = new FabricVertex(glm::vec3(currX, 0, y), length);
            this->vertices.push_back(curr);
            //std::cout << "(" << idxCoords.x << ", " << idxCoords.y << "): " << currX << ";" << y << "| " << (currX - vertices[start].x)/length.x << ", " << (y - minY) / length.y << std::endl;
            tmpArr[idxCoords.x][idxCoords.y] = curr;
            amount++;
        }
        currX += length.x;
    }
    std::cout << amount << std::endl;
    // Connect them all
    for (int i = 0; i < xVals; i++)
    {
        std::cout << " | ";
        for (int j = 0; j < yVals; j++)
        {

            FabricVertex* curr = tmpArr[i][j];
            std::cout << ((curr == nullptr) ? " " : "X") << " | ";
            if (curr == nullptr) continue;
            FabricVertex* n = nullptr;
            //Four neighbors:
            /*if (i - 1 >= 0) {
                n = tmpArr[i - 1][j];
                if (n != nullptr) {
                    curr->negWarp = n;
                    n->posWarp = curr;
                }
            }*/
            if (i + 1 < xVals) {
                n = tmpArr[i + 1][j];
                if (n != nullptr) {
                    curr->posWarp = n;
                    n->negWarp = curr;
                }
            }
            /*if (j - 1 >= 0) {
                n = tmpArr[i][j - 1];
                if (n != nullptr) {
                    curr->negWeft = n;
                    n->posWeft = curr;
                }
            }*/
            if (j + 1 < yVals) {
                n = tmpArr[i][j + 1];
                if (n != nullptr) {
                    curr->posWeft = n;
                    n->negWeft = curr;
                }
            }
        }
        std::cout << std::endl;
    }

    // Clean up array
    for (int i = 0; i < xVals; i++)
    {
        tmpArr.at(i).clear();
    }
    tmpArr.clear();
}

PolyFabric::~PolyFabric()
{
    Fabric::~Fabric();
    for (auto v : *this)
    {
        v->_reset();
        delete v;
        v = nullptr;
    }
    vertices.clear();
}

std::size_t PolyFabric::size() const
{
    return amount;
}

void PolyFabric::reset(glm::vec3 pos)
{
    this->position = pos;
    for (auto v : *this)
    {
        v->_reset();
    }
}

FabricVertex*& PolyFabric::next(std::size_t idx)
{
    // Iterated through left->right up->down
    return vertices[idx];
}

void PolyFabric::create()
{
    std::vector<glm::vec4> faceVertices;
    std::vector<glm::vec4> faceNormals;
    std::vector<glm::vec2> uvs;
    std::vector<GLuint> idx;
    {
        int curr = 0;
        for (auto v : *this)
        {
            FabricVertex* v1 = v;
            FabricVertex* v2 = v->posWarp;
            FabricVertex* v3 = v2 == nullptr ? nullptr : v2->posWeft;
            FabricVertex* v4 = v->posWeft;

            // If not all four vertices exist, there is no face to push
            if (v1 == nullptr || v2 == nullptr || v3 == nullptr || v4 == nullptr)
                continue;

            // Push pos
            faceVertices.push_back(glm::vec4(v4->localPos + this->position, 1));
            faceVertices.push_back(glm::vec4(v3->localPos + this->position, 1));
            faceVertices.push_back(glm::vec4(v2->localPos + this->position, 1));
            faceVertices.push_back(glm::vec4(v1->localPos + this->position, 1));
            // Calculate normal
            glm::vec3 ca = v2->localPos - v1->localPos;
            glm::vec3 cb = v4->localPos - v1->localPos;
            glm::vec3 nor = glm::cross(cb, ca); //Might be other way around
            // Push normals
            faceNormals.push_back(glm::vec4(nor, 0));
            faceNormals.push_back(glm::vec4(nor, 0));
            faceNormals.push_back(glm::vec4(nor, 0));
            faceNormals.push_back(glm::vec4(nor, 0));
            // Push Idx (counterclockwise!)
            idx.push_back(curr * 4);
            idx.push_back(curr * 4 + 1);
            idx.push_back(curr * 4 + 2);
            idx.push_back(curr * 4);
            idx.push_back(curr * 4 + 2);
            idx.push_back(curr * 4 + 3);
            // Calculate UV length (assuming single texture)
            //glm::vec2 subUV = glm::vec2(1.0, 1.0) / (amount - glm::vec2(1, 1));
            //// Push UVs
            //glm::vec2 uv4((i + 1) * subUV.x, 1 - j * subUV.y);
            //glm::vec2 uv3((i + 1) * subUV.x, 1 - (j + 1) * subUV.y);
            //glm::vec2 uv2(i * subUV.x, 1 - (j + 1) * subUV.y);
            //glm::vec2 uv1(i * subUV.x, 1 - j * subUV.y);

            glm::vec2 a(0, 0);
            glm::vec2 b(0, 1);
            glm::vec2 c(1, 0);
            glm::vec2 d(1, 1);

            /*uvs.push_back(uv2);
            uvs.push_back(uv3);
            uvs.push_back(uv4);
            uvs.push_back(uv1);*/
            uvs.push_back(a);
            uvs.push_back(b);
            uvs.push_back(c);
            uvs.push_back(d);

            curr++;
        }
    }
    count = idx.size();
    generateIdx();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    glBufferData(GL_ARRAY_BUFFER, faceVertices.size() * sizeof(glm::vec4), faceVertices.data(), GL_STATIC_DRAW);

    generateNor();
    glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    glBufferData(GL_ARRAY_BUFFER, faceNormals.size() * sizeof(glm::vec4), faceNormals.data(), GL_STATIC_DRAW);

    generateUV();
    glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
}