//
//  RenderTest.cpp
//  nativeGraphics
//
//  Created by Ling-Ling Zhang on 6/2/13.
//  Copyright (c) 2013 Ling-Ling Zhang. All rights reserved.
//

#include "RenderDestructible.h"
#include "glsl_helper.h"
#include "transform.h"
#include "common.h"
#include "log.h"

#include "Vector3.h"
#include "Point3.h"
#include "obj_parser.h"

#define voxelSize 25.0

struct DestructibleNode
{
    GLfloat position[3];
    Vector3 velocity;
    vector<struct DestructibleBond *> bonds;
};

struct DestructibleFace
{
    DestructibleFace(DestructibleNode *node1, DestructibleNode *node2, DestructibleNode *node3)
    {
        nodes.push_back(node1);
        nodes.push_back(node2);
        nodes.push_back(node3);
    }
    vector<DestructibleNode *> nodes;
    DestructibleCell *cell;
};

struct DestructibleBond
{
    vector<DestructibleNode *> nodes;
    GLfloat origLength;
    GLfloat springConst;
    GLfloat dampConst;
    GLfloat breakThresh;
    bool broken;
};

struct DestructibleCell
{
    vector<struct DestructibleBond *> bonds;
    vector<struct DestructibleNode *> nodes;
    bool broken;
    int xidx, yidx, zidx;
};

static struct DestructibleCell* createCell(float xmin, float ymin, float zmin);
static struct DestructibleNode *createNode(float x, float y, float z);
static struct DestructibleBond *createBond(DestructibleNode *node1, DestructibleNode *node2);
static struct DestructibleFace *createFace(DestructibleNode *node1, DestructibleNode *node2, DestructibleNode *node3, DestructibleCell *cell);

std::vector<struct DestructibleCell *> cells;
std::vector<struct DestructibleNode *> nodes;
std::vector<struct DestructibleBond *> bonds;
std::vector<struct DestructibleFace *> surfaces;

RenderDestructible::RenderDestructible(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename) : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename) {
    
    std::vector<struct Vertex> vertices;
    std::vector<struct face> faces;
    getObjectData((char *)resourceCallback(objFilename), numVertices, vertices, faces, false, false);
    
    float xmax = -100;
    float ymax = -100;
    float zmax = -100;
    float xmin = 100;
    float ymin = 100;
    float zmin = 100;
    
    for (int i = 0; i < vertices.size(); i++) {
        struct Vertex vert = vertices[i];
        if (vert.coord.x > xmax)
            xmax = vert.coord.x;
        if (vert.coord.x < xmin)
            xmin = vert.coord.x;
        if (vert.coord.y > ymax)
            ymax = vert.coord.y;
        if (vert.coord.y < ymin)
            ymin = vert.coord.y;
        if (vert.coord.z > zmax)
            zmax = vert.coord.z;
        if (vert.coord.z < zmin)
            zmin = vert.coord.z;
    }
    
    int zrows = ceil((zmax - zmin)/voxelSize) + 1;
    int yrows = ceil((ymax - ymin)/voxelSize) + 1;
    int xrows = ceil((xmax - xmin)/voxelSize) + 1;
    
    //Set up voxel grid
    voxelGrid3D = (int *) malloc(xrows * sizeof(int *));
    
    for (int i = 0; i < xrows; i++) {
        int *voxelGrid2D = (int *) malloc(yrows * sizeof(int *));
        ((int **)voxelGrid3D)[i] = voxelGrid2D;
        
        for (int j = 0; j < yrows; j++) {
            int *voxelRow = (int *) malloc(zrows * sizeof(int));
            for (int k = 0; k < zrows; k++) {
                voxelRow[k] = 0;
            }
            ((int **)voxelGrid2D)[j] = voxelRow;
        }
    }
    
    for(int i = 0; i < faces.size(); i++) {
        struct face cur_face = faces[i];
        struct Vertex v0 = vertices[cur_face.vertex[0]];
        struct Vertex v1 = vertices[cur_face.vertex[1]];
        struct Vertex v2 = vertices[cur_face.vertex[2]];
        
        float xstart = v0.coord.x;
        float xend = v0.coord.x;
        float ystart = v0.coord.y;
        float yend = v0.coord.y;
        float zstart = v0.coord.z;
        float zend = v0.coord.z;
        
        if (v1.coord.x < xstart)
            xstart = v1.coord.x;
        if (v1.coord.x > xend)
            xend = v1.coord.x;
        if (v2.coord.x < xstart)
            xstart = v2.coord.x;
        if (v2.coord.x > xend)
            xend = v2.coord.x;
        
        if (v1.coord.y < ystart)
            ystart = v1.coord.y;
        if (v1.coord.y > yend)
            yend = v1.coord.y;
        if (v2.coord.y < ystart)
            ystart = v2.coord.y;
        if (v2.coord.y > yend)
            yend = v2.coord.y;
        
        if (v1.coord.z < zstart)
            zstart = v1.coord.z;
        if (v1.coord.z > zend)
            zend = v1.coord.z;
        if (v2.coord.z < zstart)
            zstart = v2.coord.z;
        if (v2.coord.z > zend)
            zend = v2.coord.z;
        
        int xstart_idx = floor((xstart - xmin)/voxelSize);
        int xend_idx = ceil((xend - xmin)/voxelSize);
        int ystart_idx = floor((ystart - ymin)/voxelSize);
        int yend_idx = ceil((yend - ymin)/voxelSize);
        int zstart_idx = floor((zstart - zmin)/voxelSize);
        int zend_idx = ceil((zend - zmin)/voxelSize);
        
        /*if (xstart_idx == xend_idx)
            xend_idx += 1;
        if (ystart_idx == yend_idx)
            yend_idx += 1;
        if (zstart_idx == zend_idx)
            zend_idx += 1;
        */
        for (int xdim = xstart_idx; xdim <= xend_idx; xdim++) {
            for (int ydim = ystart_idx; ydim <= yend_idx; ydim++) {
                for (int zdim = zstart_idx; zdim <= zend_idx; zdim++) {
                    //if the voxel intersects the face
                    ((int*)((int **)voxelGrid3D[xdim])[ydim])[zdim] = i+1;
                }
            }
        }
    }
    
    
    for (int xdim = 0; xdim < xrows; xdim++) {
        for (int ydim = 0; ydim < yrows; ydim++) {
            bool down = false;
            int old_face = 0;
            for (int zdim = 0; zdim < zrows; zdim++) {
                int cur_face = ((int*)((int **)voxelGrid3D[xdim])[ydim])[zdim];
                if (cur_face > 0) {
                    if (cur_face != old_face) {
                        down = !down;
                        old_face = cur_face;
                    }
                }
                else if (down) {
                    ((int*)((int **)voxelGrid3D[xdim])[ydim])[zdim] = -1;
                } else
                    continue;
                cells.push_back(createCell(xmin + ((float)xdim * voxelSize), ymin + ((float)ydim * voxelSize), zmin + ((float)zdim) * voxelSize));
            }
        }
    }
}

static struct DestructibleCell* createCell(float xmin, float ymin, float zmin) {
    DestructibleCell *cell = new DestructibleCell;
    
    DestructibleNode *node1 = createNode(xmin, ymin, zmin);
    DestructibleNode *node2 = createNode(xmin + voxelSize, ymin + voxelSize, zmin + voxelSize);
    DestructibleBond *bond1 = createBond(node1, node2);
    node1->bonds.push_back(bond1);
    node2->bonds.push_back(bond1);
    
    DestructibleNode *node3 = createNode(xmin + voxelSize, ymin, zmin);
    DestructibleNode *node4 = createNode(xmin, ymin + voxelSize, zmin + voxelSize);
    DestructibleBond *bond2 = createBond(node3, node4);
    node3->bonds.push_back(bond2);
    node4->bonds.push_back(bond2);
    
    DestructibleNode *node5 = createNode(xmin, ymin + voxelSize, zmin);
    DestructibleNode *node6 = createNode(xmin + voxelSize, ymin, zmin + voxelSize);
    DestructibleBond *bond3 = createBond(node5, node6);
    node5->bonds.push_back(bond3);
    node6->bonds.push_back(bond3);
    
    DestructibleNode *node7 = createNode(xmin, ymin, zmin + voxelSize);
    DestructibleNode *node8 = createNode(xmin + voxelSize, ymin + voxelSize, zmin);
    DestructibleBond *bond4 = createBond(node7, node8);
    node7->bonds.push_back(bond4);
    node8->bonds.push_back(bond4);
    
    cell->nodes.push_back(node1);
    cell->nodes.push_back(node2);
    cell->nodes.push_back(node3);
    cell->nodes.push_back(node4);
    cell->nodes.push_back(node5);
    cell->nodes.push_back(node6);
    cell->nodes.push_back(node7);
    cell->nodes.push_back(node8);
    
    cell->bonds.push_back(bond1);
    cell->bonds.push_back(bond2);
    cell->bonds.push_back(bond3);
    cell->bonds.push_back(bond4);
    
    struct DestructibleFace * face1 = createFace(node1, node5, node3, cell);
    struct DestructibleFace * face2 = createFace(node3, node5, node8, cell);
    struct DestructibleFace * face3 = createFace(node6, node7, node1, cell);
    struct DestructibleFace * face4 = createFace(node6, node1, node3, cell);
    struct DestructibleFace * face5 = createFace(node1, node7, node5, cell);
    struct DestructibleFace * face6 = createFace(node5, node7, node4, cell);
    struct DestructibleFace * face7 = createFace(node3, node8, node6, cell);
    struct DestructibleFace * face8 = createFace(node6, node8, node2, cell);
    struct DestructibleFace * face9 = createFace(node5, node4, node8, cell);
    struct DestructibleFace * face10 = createFace(node8, node4, node2, cell);
    struct DestructibleFace * face11 = createFace(node4, node7, node6, cell);
    struct DestructibleFace * face12 = createFace(node6, node2, node4, cell);
    
    surfaces.push_back(face1);
    surfaces.push_back(face2);
    surfaces.push_back(face3);
    surfaces.push_back(face4);
    surfaces.push_back(face5);
    surfaces.push_back(face6);
    surfaces.push_back(face7);
    surfaces.push_back(face8);
    surfaces.push_back(face9);
    surfaces.push_back(face10);
    surfaces.push_back(face11);
    surfaces.push_back(face12);
    
    cell->broken = false;
    return cell;
}

static struct DestructibleFace *createFace(DestructibleNode *node1, DestructibleNode *node2, DestructibleNode *node3, DestructibleCell *cell)
{
    struct DestructibleFace * face = new DestructibleFace(node1, node2, node3);
    face->cell = cell;
    return face;
}

static struct DestructibleBond *createBond(DestructibleNode *node1, DestructibleNode *node2)
{
    DestructibleBond *bond = new DestructibleBond;
    bond->nodes.push_back(node1);
    bond->nodes.push_back(node2);
    
    bond->origLength = sqrt(pow(node1->position[0]-node2->position[0],2) + pow(node1->position[1]-node2->position[1],2) + pow(node1->position[2]-node2->position[2],2));
    bond->breakThresh = (GLfloat)(rand() % 100)/100;
    bond->springConst = 25;
    bond->broken = false;
    bond->dampConst = .05;
    bonds.push_back(bond);
    
    return bond;
}

static struct DestructibleNode *createNode(float x, float y, float z)
{
    for (int i = 0; i < nodes.size(); i++) {
        DestructibleNode *node = nodes[i];
        if (node->position[0] == x && node->position[1] == y && node->position[2] == z)
            return node;
    }
    DestructibleNode *node = new DestructibleNode;
    node->position[0] = x;
    node->position[1] = y;
    node->position[2] = z;
    node->velocity = Vector3();
    nodes.push_back(node);
    
    return node;
}

bool explode = false;

void RenderDestructible::RenderPass() {

    if (!explode) {
        nodes[0]->velocity = Vector3(1.0, 1.0, 1.0);
        explode = true;
    }
    
    for (int node_idx = 0; node_idx < nodes.size(); node_idx++) {
        DestructibleNode *node = nodes[node_idx];
        Vector3 force = Vector3();
        
        //Internal forces
        for (int bond_idx = 0; bond_idx < node->bonds.size(); bond_idx++) {
            DestructibleBond *bond = node->bonds[bond_idx];
            
            if (bond->broken) {
                node->bonds.erase(node->bonds.begin() + bond_idx);
                bond_idx--;
                continue;
            }
            
            DestructibleNode *node1 = bond->nodes[0];
            DestructibleNode *node2 = bond->nodes[1];
            
            Vector3 vec = Vector3(node1->position[0] - node2->position[0], node1->position[1] - node2->position[1], node1->position[2] - node2->position[2]);
            float bondLength = sqrt(pow(node1->position[0]-node2->position[0],2) + pow(node1->position[1]-node2->position[1],2) + pow(node1->position[2]-node2->position[2],2));
            vec.Normalize();
            vec = vec * (bondLength - bond->origLength) * bond->springConst;
            
            Vector3 vel = (node1->velocity - node2->velocity) * bond->dampConst;
            
            if (node->position[0] == node2->position[0] && node->position[1] == node2->position[1] && node->position[2] == node2->position[2]) {
                vec *= -1;
                vel *= -1;
            }
            
            force = force + vec + vel;
        }
        Vector3 accel = force / 10.0;
        node->velocity = node->velocity + (accel * .01);
        node->position[0] = node->position[0] + (node->velocity.x * .01);
        node->position[1] = node->position[1] + (node->velocity.y * .01);
        node->position[2] = node->position[2] + (node->velocity.z * .01);
        
    }
    
    //Check for broken cells
    for (int i = 0; i < cells.size(); i++)
    {
        DestructibleCell *cell = cells[i];
        if (cell->broken)
            continue;
            
        for (int j = 0; j < cell->bonds.size(); j++) {
            DestructibleBond *bond = cell->bonds[j];
            DestructibleNode *node1 = bond->nodes[0];
            DestructibleNode *node2 = bond->nodes[1];
            float bondLength = sqrt(pow(node1->position[0]-node2->position[0],2) + pow(node1->position[1]-node2->position[1],2) + pow(node1->position[2]-node2->position[2],2));
            if (bondLength/bond->origLength > 1.0 + bond->breakThresh) {
                cell->broken = true;
                for (int k = 0; k < cell->bonds.size(); k++) {
                    cell->bonds[k]->broken = true;
                }
            }
        }
    }
    
    //Find surface faces
    std::vector<struct DestructibleNode *> surfaceNodes;
    int numSurfaceVertices = 0;
    
    for (int face_idx = 0; face_idx < surfaces.size(); face_idx++) {
        struct DestructibleFace * face = surfaces[face_idx];
        if (face->cell->broken) {
            surfaces.erase(surfaces.begin() + face_idx);
            face_idx--;
            continue;
        }
        
        bool surface = true;
        for (int node_idx = 0; node_idx < face->nodes.size(); node_idx++) {
            if (face->nodes[node_idx]->bonds.size() > 4) {
                surface = false;
                break;
            }
        }
        if (surface == true) {
            numSurfaceVertices += 3;
            surfaceNodes.push_back(face->nodes[0]);
            surfaceNodes.push_back(face->nodes[1]);
            surfaceNodes.push_back(face->nodes[2]);
        }
    }
    
    GLfloat * vertexBuffer = (float *)malloc(numSurfaceVertices * (3+3+2) * sizeof(float));
    int bufferIndex = 0;
    for (int node_idx = 0; node_idx < surfaceNodes.size(); node_idx++) {
        vertexBuffer[bufferIndex++] = surfaceNodes[node_idx]->position[0];
        vertexBuffer[bufferIndex++] = surfaceNodes[node_idx]->position[1];
        vertexBuffer[bufferIndex++] = surfaceNodes[node_idx]->position[2];
        vertexBuffer[bufferIndex++] = 0.0;
        vertexBuffer[bufferIndex++] = 0.0;
        vertexBuffer[bufferIndex++] = 0.0;
        vertexBuffer[bufferIndex++] = 0.0;
        vertexBuffer[bufferIndex++] = 0.0;
    }
    
    numVertices = numSurfaceVertices;
    
    //Create vbo
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, numVertices * (3+3+2) * sizeof(float), vertexBuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGlError("VertexBuffer Generation");
    free(vertexBuffer);
     
    
    // Pass matrices
    GLfloat* mv_Matrix = (GLfloat*)mvMatrix();
    GLfloat* mvp_Matrix = (GLfloat*)mvpMatrix();
    glUniformMatrix4fv(gmvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    glUniformMatrix4fv(gmvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    checkGlError("glUniformMatrix4fv");
    delete mv_Matrix;
    delete mvp_Matrix;
    
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    
    // Pass vertices
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) 0);
    checkGlError("gvPositionHandle");
    /*
     // Pass normals
     if(gvNormals != -1) {
     glEnableVertexAttribArray(gvNormals);
     glVertexAttribPointer(gvNormals, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) (3 * sizeof(GLfloat)));
     checkGlError("gvNormals");
     }
     
     // Pass texture coords
     if(gvTexCoords != -1) {
     glEnableVertexAttribArray(gvTexCoords);
     glVertexAttribPointer(gvTexCoords, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid *) (6 * sizeof(GLfloat)));
     checkGlError("gvTexCoords");
     }
     */
    // Pass texture
    if(textureUniform != -1 && texture != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureUniform, 0);
        checkGlError("texture");
    }
    
    // Pass normal map
    if(normalMapUniform != -1 && normalTexture != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTexture);
        glUniform1i(textureUniform, 1);
        checkGlError("normalTexture");
    }
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
}

void RenderDestructible::Render() {
    
    if(!pipeline) {
        LOGE("RenderPipeline inaccessible.");
        exit(0);
    }
    
    //////////////////////////////////
    // Render to frame buffer
    
    // Render colors (R, G, B, UNUSED / SPECULAR)
    SetShader(colorShader);
    
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline->frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipeline->depthBuffer);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL); // TODO: Measure effect on performance vs clear buffer.
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    checkGlError("glClear");
    
    RenderPass();
    
    // Render geometry (NX_MV, NY_MV, NZ_MV, Depth_MVP)
    SetShader(pipeline->geometryShader);
    
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline->frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->geometryTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipeline->depthBuffer);
    
    glDepthMask(GL_FALSE); // We share the same depth buffer here, so don't overwrite it.
    glDepthFunc(GL_EQUAL);
    glEnable(GL_DITHER);
    
    RenderPass();
    
    glDepthMask(GL_TRUE); // TODO
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); // TODO: unbind other resources
}