#include "Mesh.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <list>

TruncatedPyramid::TruncatedPyramid(glm::vec3 orig, glm::vec2 baseHalfDims, float pyrH, float truncH)
    : o(orig), bhd(baseHalfDims), ph(pyrH), th(truncH),
    top(orig + glm::vec3(0, pyrH, 0)),
    A(orig + glm::vec3(-baseHalfDims.x, 0, baseHalfDims.y)),
    B(orig + glm::vec3(baseHalfDims.x, 0, baseHalfDims.y)),
    C(orig + glm::vec3(baseHalfDims.x, 0, -baseHalfDims.y)),
    D(orig + glm::vec3(-baseHalfDims.x, 0, -baseHalfDims.y))
{
    glm::vec3 normFront = glm::normalize(glm::cross(top - B, A - B));
    glm::vec3 normRight = glm::normalize(glm::cross(top - C, B - C));
    glm::vec3 normBack = glm::normalize(glm::cross(top - D, C - D));
    glm::vec3 normLeft = glm::normalize(glm::cross(top - A, D - A));

    planeFront = glm::vec4(normFront, glm::dot(normFront, A));
    planeRight = glm::vec4(normRight, glm::dot(normRight, B));
    planeBack = glm::vec4(normBack, glm::dot(normBack, C));
    planeLeft = glm::vec4(normLeft, glm::dot(normLeft, D));
}

glm::vec3 TruncatedPyramid::nearestNormal(glm::vec3 point)
{
    glm::vec4 planeToPointF = glm::vec4(point - A, 0);
    glm::vec4 planeToPointR = glm::vec4(point - B, 0);
    glm::vec4 planeToPointB = glm::vec4(point - C, 0);
    glm::vec4 planeToPointL = glm::vec4(point - D, 0);
    float dotF = glm::abs(glm::dot(planeFront, planeToPointF));
    float dotR = glm::abs(glm::dot(planeFront, planeToPointR));
    float dotB = glm::abs(glm::dot(planeFront, planeToPointB));
    float dotL = glm::abs(glm::dot(planeFront, planeToPointL));
    float nearestDot = dotF;
    glm::vec4 nearest = planeFront;
    if (dotR < nearestDot) {
        nearest = planeRight;
        nearestDot = dotR;
    }
    if (dotB < nearestDot) {
        nearest = planeBack;
        nearestDot = dotB;
    }
    if (dotL < nearestDot) {
        nearest = planeLeft;
        nearestDot = dotL;
    }
    if (point.y - (th + o.y) < nearestDot) {
        return glm::vec3(0, 1, 0);
    }
    //TODO: Check bottom too even if irrelevant right now
    return glm::vec3(nearest);
}

bool TruncatedPyramid::isInBounds(glm::vec3 point)
{
    // Function that returns the scale of the base based on the y coord of point
    float scale = (ph - (point.y - o.y)) / ph; // Fraction of pyr. TODO add origin 
    return (point.y >= o.y && point.y <= th)/*y coord*/
        && (point.x >= (o.x - scale * bhd.x) && point.x <= (o.x + scale * bhd.x))/*x coord*/
        && (point.z >= (o.z - scale * bhd.y) && point.z <= (o.z + scale * bhd.y))/*z coord*/;
}

void printVec4(glm::vec4 v, std::string prefix = "") {
    std::cout << prefix << ": (" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")" << std::endl;
}

//void TruncatedPyramid::create()
//{
//    glm::vec4 A(this->A, 1), B(this->B, 1), C(this->C, 1), D(this->D, 1), Top(this->top, 1);
//    std::cout << "=== TRUNCATED PYRAMID ===" << std::endl;
//    printVec4(A, "A");
//    printVec4(B, "B");
//    printVec4(C, "C");
//    printVec4(D, "D");
//    printVec4(Top, "T");
//    std::cout << "=========================" << std::endl;
//    glm::vec4 nF(planeFront.x, planeFront.y, planeFront.z, 0),
//        nR(planeRight.x, planeRight.y, planeRight.z, 0),
//        nB(planeBack.x, planeBack.y, planeBack.z, 0),
//        nL(planeLeft.x, planeLeft.y, planeLeft.z, 0),
//        nBot(0, -1, 0, 0);
//    std::vector<glm::vec4> pos{//Front
//                                A, B, Top,
//                                //Right
//                                B, C, Top,
//                                //Back
//                                C, D, Top,
//                                //Left
//                                D, A, Top,
//                                //Bottom
//                                A, D, C, B
//    };
//    std::vector<glm::vec4> nor{ nF, nF, nF, nR, nR, nR, nB, nB, nB, nL, nL, nL, nBot, nBot, nBot, nBot };
//    std::vector<glm::vec2> uvs{
//                    glm::vec2(0.1, 0.9),
//                    glm::vec2(.2, 0.9),
//                    glm::vec2(.2, 1),
//                    //glm::vec2(.1, 1),
//
//                    glm::vec2(.3, .9),
//                    glm::vec2(.3, 1),
//                    glm::vec2(.2, 1),
//                    //glm::vec2(.2, .9),
//
//                    glm::vec2(.4, .9),
//                    glm::vec2(.3, .9),
//                    glm::vec2(.3, 1),
//                    //glm::vec2(.4, 1),
//
//                    glm::vec2(.2, .8),
//                    glm::vec2(.2, .9),
//                    glm::vec2(.1, .9),
//                    //glm::vec2(.1, .8),
//
//                    //glm::vec2(0, .8),
//                   // glm::vec2(0, .9),
//                    //glm::vec2(.1, .9),
//                   // glm::vec2(.1, .8),
//
//                    glm::vec2(.2, .9),
//                    glm::vec2(.2, .8),
//                    glm::vec2(.3, .8),
//                    glm::vec2(.3, .9)
//    };
//
//    std::vector<GLuint> idx{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 12, 14, 15};
//
//    count = 18;
//    generateIdx();
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);
//
//    generatePos();
//    glBindBuffer(GL_ARRAY_BUFFER, bufPos);
//    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);
//
//    generateNor();
//    glBindBuffer(GL_ARRAY_BUFFER, bufNor);
//    glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);
//
//    generateUV();
//    glBindBuffer(GL_ARRAY_BUFFER, bufUV);
//    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
//}

void TruncatedPyramid::create()
{
    float scale = (ph - th) / ph;
    glm::vec4 A(this->A, 1), B(this->B, 1), C(this->C, 1), D(this->D, 1), Top(this->top, 1),
        E(o.x - scale * bhd.x, o.y + th, o.z + scale * bhd.y, 1),
        F(o.x + scale * bhd.x, o.y + th, o.z + scale * bhd.y, 1),
        G(o.x + scale * bhd.x, o.y + th, o.z - scale * bhd.y, 1),
        H(o.x - scale * bhd.x, o.y + th, o.z - scale * bhd.y, 1);

    std::cout << "=== TRUNCATED PYRAMID ===" << std::endl;
    printVec4(A, "A");
    printVec4(B, "B");
    printVec4(C, "C");
    printVec4(D, "D");
    printVec4(Top, "T");
    std::cout << "=========================" << std::endl;
    glm::vec4 nF(planeFront.x, planeFront.y, planeFront.z, 0),
        nR(planeRight.x, planeRight.y, planeRight.z, 0),
        nB(planeBack.x, planeBack.y, planeBack.z, 0),
        nL(planeLeft.x, planeLeft.y, planeLeft.z, 0),
        nBot(0, -1, 0, 0), nTop(0, 1, 0, 0);

    std::vector<glm::vec4> pos{//Front
                                A, B, F, E,
                                //Left
                                A, E, H, D,
                                //Back
                                D, C, G, H,
                                //Right
                                C, G, F, B,
                                //Top
                                E, F, G, H,
                                //Bottom
                                A, D, C, B
    };
    std::vector<glm::vec4> nor{
        nF, nF, nF, nF,
        nL, nL, nL, nL,
        nB, nB, nB, nB,
        nR, nR, nR, nR,
        nTop, nTop, nTop, nTop,
        nBot, nBot, nBot, nBot
    };

    std::vector<glm::vec2> uvs{
                    glm::vec2(0.1, 0.9),
                    glm::vec2(.2, 0.9),
                    glm::vec2(.2, 1),
                    glm::vec2(.1, 1),

                    glm::vec2(.3, .9),
                    glm::vec2(.3, 1),
                    glm::vec2(.2, 1),
                    glm::vec2(.2, .9),

                    glm::vec2(.4, .9),
                    glm::vec2(.3, .9),
                    glm::vec2(.3, 1),
                    glm::vec2(.4, 1),

                    glm::vec2(.2, .8),
                    glm::vec2(.2, .9),
                    glm::vec2(.1, .9),
                    glm::vec2(.1, .8),

                    glm::vec2(0, .8),
                    glm::vec2(0, .9),
                    glm::vec2(.1, .9),
                    glm::vec2(.1, .8),

                    glm::vec2(.2, .9),
                    glm::vec2(.2, .8),
                    glm::vec2(.3, .8),
                    glm::vec2(.3, .9)
    };

    std::vector<GLuint> idx{ 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 11, 10, 8, 10, 9,
                             12, 13, 14, 12, 14, 15, 16, 19, 18, 16, 18, 17, 20, 21, 22, 20, 22, 23 };


    count = 36;
    generateIdx();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateNor();
    glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

    generateUV();
    glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);

}

Cube::Cube(glm::vec3 orig, glm::vec3 halfDims, float angle, glm::vec3 axis)
    : Drawable(), Collidable(), origin(orig), halfLengths(halfDims)
{
    this->rot = glm::rotate(angle, axis);
    this->invrot = glm::inverse(this->rot);
}

bool Cube::isInBounds(glm::vec3 point)
{
    glm::vec4 p = invrot * glm::vec4(point, 1);
    return p.x <= 1 && p.x >= -1 && p.y <= 1 && p.y >= -1 && p.z <= 1 && p.z >= -1;
}

glm::vec3 Cube::nearestNormal(glm::vec3 point)
{
    glm::vec4 p = invrot * glm::vec4(point, 1);
    //Distance Y
    float dst_top = (origin.y + halfLengths.y) - p.y;
    float dst_bot = p.y - (origin.y - halfLengths.y);
    float dst_y = std::min(dst_top, dst_bot);
    float yDir = dst_y == dst_top ? 1 : -1;
    //Distance X
    float dst_rgt = (origin.x + halfLengths.x) - p.x;
    float dst_lft = p.x - (origin.x - halfLengths.x);
    float dst_x = std::min(dst_rgt, dst_lft);
    float xDir = dst_x == dst_rgt ? 1 : -1;
    //Distance Z
    float dst_frt = (origin.z + halfLengths.z) - p.z;
    float dst_bck = p.z - (origin.z - halfLengths.z);
    float dst_z = std::min(dst_frt, dst_bck);
    float zDir = dst_z == dst_frt ? 1 : -1;
    // Total min (What if mins are equal? Should not happen if timesteps aren't too big...)
    float min = std::min(std::min(dst_x, dst_y), dst_z);
    if (min == dst_y) return rot * (yDir * glm::vec4(0, 1, 0, 0));
    if (min == dst_x) return rot * (xDir * glm::vec4(1, 0, 0, 0));
    if (min == dst_z) return rot * (zDir * glm::vec4(0, 0, 1, 0));


    //float dst_top = 1 - p.y;
    //float dst_right = 1 - p.x;
    //float dst_front = 1 - p.z; //Dont do other 3 since theyll never collide in curr setup
    //float min = std::min(std::min(dst_top, dst_right), dst_front);
    //if (min == dst_top) return glm::vec3(0, 1, 0);
    //if (min == dst_right) return glm::vec3(1, 0, 0);
    //if (min == dst_front) return glm::vec3(0, 0, 1);
    return glm::vec3(0, 0, 0);
}

void Cube::create()
{
    //std::vector<glm::vec4> verts{
    //    //Front
    //    glm::vec4(origin - halfLengths.x, origin - halfLengths.y, origin + halfLengths.z, 1),
    //    glm::vec4(origin + halfLengths.x, origin - halfLengths.y, origin + halfLengths.z, 1),
    //    glm::vec4(origin + halfLengths.x, origin + halfLengths.y, origin + halfLengths.z, 1),
    //    glm::vec4(origin - halfLengths.x, origin + halfLengths.y, origin + halfLengths.z, 1),
    //    //Left
    //    glm::vec4(origin - halfLengths.x, origin - halfLengths.y, origin + halfLengths.z, 1),
    //    glm::vec4(origin - halfLengths.x, origin - halfLengths.y, origin + halfLengths.z, 1),
    //    glm::vec4(origin - halfLengths.x, origin - halfLengths.y, origin + halfLengths.z, 1),
    //    glm::vec4(origin - halfLengths.x, origin - halfLengths.y, origin + halfLengths.z, 1),
    //};
    std::vector<glm::vec4> pos{//Front
                                glm::vec4(-1, -1, 1, 1),
                                glm::vec4(1, -1, 1, 1),
                                glm::vec4(1, 1, 1, 1),
                                glm::vec4(-1, 1, 1, 1),
                                //Left
                                glm::vec4(-1, -1, 1, 1),
                                glm::vec4(-1, 1, 1, 1),
                                glm::vec4(-1, 1, -1, 1),
                                glm::vec4(-1, -1, -1, 1),
                                //Back
                                glm::vec4(-1, -1, -1, 1),
                                glm::vec4(1, -1, -1, 1),
                                glm::vec4(1, 1, -1, 1),
                                glm::vec4(-1, 1, -1, 1),
                                //Right
                                glm::vec4(1, -1, -1, 1),
                                glm::vec4(1, 1, -1, 1),
                                glm::vec4(1, 1, 1, 1),
                                glm::vec4(1, -1, 1, 1),
                                //Top
                                glm::vec4(-1, 1, 1, 1),
                                glm::vec4(-1, 1, -1, 1),
                                glm::vec4(1, 1, -1, 1),
                                glm::vec4(1, 1, 1, 1),
                                //Bottom
                                glm::vec4(-1, -1, 1, 1),
                                glm::vec4(-1, -1, -1, 1),
                                glm::vec4(1, -1, -1, 1),
                                glm::vec4(1, -1, 1, 1) };
    std::vector<glm::vec4> nor{ glm::vec4(0, 0, 1, 0),
                                glm::vec4(0, 0, 1, 0),
                                glm::vec4(0, 0, 1, 0),
                                glm::vec4(0, 0, 1, 0),

                                glm::vec4(-1, 0, 0, 0),
                                glm::vec4(-1, 0, 0, 0),
                                glm::vec4(-1, 0, 0, 0),
                                glm::vec4(-1, 0, 0, 0),

                                glm::vec4(0, 0, -1, 0),
                                glm::vec4(0, 0, -1, 0),
                                glm::vec4(0, 0, -1, 0),
                                glm::vec4(0, 0, -1, 0),

                                glm::vec4(1, 0, 0, 0),
                                glm::vec4(1, 0, 0, 0),
                                glm::vec4(1, 0, 0, 0),
                                glm::vec4(1, 0, 0, 0),

                                glm::vec4(0, 1, 0, 0),
                                glm::vec4(0, 1, 0, 0),
                                glm::vec4(0, 1, 0, 0),
                                glm::vec4(0, 1, 0, 0),

                                glm::vec4(0, -1, 0, 0),
                                glm::vec4(0, -1, 0, 0),
                                glm::vec4(0, -1, 0, 0),
                                glm::vec4(0, -1, 0, 0) };

    for (int i = 0; i < pos.size(); i++)
    {
        glm::vec4 basis = pos[i];
        pos[i] = rot * glm::vec4(origin.x + basis.x * halfLengths.x, origin.y + basis.y * halfLengths.y, origin.z + basis.z * halfLengths.z, 1);
        nor[i] = rot * nor[i];
    }


    std::vector<glm::vec2> uvs{
                    glm::vec2(0.1, 0.9),
                    glm::vec2(.2, 0.9),
                    glm::vec2(.2, 1),
                    glm::vec2(.1, 1),

                    glm::vec2(.3, .9),
                    glm::vec2(.3, 1),
                    glm::vec2(.2, 1),
                    glm::vec2(.2, .9),

                    glm::vec2(.4, .9),
                    glm::vec2(.3, .9),
                    glm::vec2(.3, 1),
                    glm::vec2(.4, 1),

                    glm::vec2(.2, .8),
                    glm::vec2(.2, .9),
                    glm::vec2(.1, .9),
                    glm::vec2(.1, .8),

                    glm::vec2(0, .8),
                    glm::vec2(0, .9),
                    glm::vec2(.1, .9),
                    glm::vec2(.1, .8),

                    glm::vec2(.2, .9),
                    glm::vec2(.2, .8),
                    glm::vec2(.3, .8),
                    glm::vec2(.3, .9)
    };

    std::vector<GLuint> idx{ 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 11, 10, 8, 10, 9,
                             12, 13, 14, 12, 14, 15, 16, 19, 18, 16, 18, 17, 20, 21, 22, 20, 22, 23 };

    std::vector<glm::vec4> tri{ glm::vec4(-.6, -.4, -.2, 1), glm::vec4(.6, -.4, 0, 1), glm::vec4(0, .6, 0, 1) };
    std::vector<GLuint> l{ 0, 1, 2 };
    count = 36;
    //count = 3;
    generateIdx();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateNor();
    glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

    generateUV();
    glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);

}

Vector::Vector(glm::vec3 o, glm::vec3 p) : origin(o), pointer(p)
{
    this->currDrawMode = GL_LINES;
}
Vector::~Vector() {}
void Vector::create()
{
    glm::vec4 pos[2] = { glm::vec4(origin, 1), glm::vec4(pointer, 1) };
    glm::vec4 col[2] = { glm::vec4(0.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 1.f) };
    GLuint idx[2] = { 0, 1 };
    count = 2;
    generateIdx();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * sizeof(GLuint), idx, GL_STATIC_DRAW);

    generatePos();
    glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec4), pos, GL_STATIC_DRAW);

    generateNor();
    glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec4), col, GL_STATIC_DRAW);
}

Vectors::Vectors(glm::vec4 originColor, glm::vec4 pointerColor)
    : startColor(originColor), endColor(pointerColor)
{
    this->currDrawMode = GL_LINES;
}
Vectors::~Vectors()
{
    clear();
}
void Vectors::addVector(glm::vec3 o, glm::vec3 p, glm::vec4 sc, glm::vec4 ec)
{
    this->idx.push_back(pointers.size());
    this->idx.push_back(pointers.size() + 1);
    this->pointers.push_back(glm::vec4(o, 1));
    this->pointers.push_back(glm::vec4(o + p, 1));
    this->colors.push_back(sc);
    this->colors.push_back(ec);
}
void Vectors::addVector(glm::vec3 o, glm::vec3 p)
{
    addVector(o, p, startColor, endColor);
}
void Vectors::clear()
{
    pointers.clear();
    colors.clear();
    idx.clear();
}
void Vectors::change(int index, glm::vec3 origin, glm::vec3 pointer)
{
    pointers[2 * index] = glm::vec4(origin, 1);
    pointers[2 * index + 1] = glm::vec4(pointer, 1);
}
void Vectors::create()
{
    count = pointers.size();
    generateIdx();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec4), pointers.data(), GL_STATIC_DRAW);

    generateNor();
    glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);

}

Axis3D::Axis3D(glm::vec3 origin, float scale) : Vectors(glm::vec4(), glm::vec4())
{
    addVector(origin, origin + glm::vec3(scale, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec4(1, 0, 0, 1.f));
    addVector(origin, origin + glm::vec3(0, scale, 0), glm::vec4(0, 1, 0, 1), glm::vec4(0, 1, 0, 1.f));
    addVector(origin, origin + glm::vec3(0, 0, scale), glm::vec4(0, 0, 1, 1.f), glm::vec4(0, 0, 1, 1.f));
}