#include "vcepch.h"
#include "VeryCoolEngine/Log.h"
#include "VeryCoolEngine/AssetHandling/Assets.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include "VeryCoolEngine/core.h"
#include "VeryCoolEngine/Components/TerrainComponent.h"
#include "VeryCoolEngine/PlatformTypes.h"
#include "VeryCoolEngine/Renderer/Mesh.h"

/*
namespace std
{
    template<>
    struct hash<glm::highp_vec3>
    {

        size_t operator()(glm::highp_vec3 const& s) const
        {
            size_t ret = 24039598597867 * 974869875987570600;

            ret ^= (*reinterpret_cast<size_t*>((void*) & (s.x)) >> 16) * 9748670600;
            ret ^= (*reinterpret_cast<size_t*>((void*) & (s.y)) >> 16) * 9748670600;
            ret ^= (*reinterpret_cast<size_t*>((void*) & (s.z)) >> 16) * 9748670600;
            return ret;
        }
    };
}
*/

namespace VeryCoolEngine {

    void AverageTwoPixels(cv::Mat& xImage, glm::vec2 xCoord1, glm::vec2 xCoord2) {
        cv::Vec3b xPixel1 = xImage.at<cv::Vec3b>(xCoord1.x, xCoord1.y);
        cv::Vec3b xPixel2 = xImage.at<cv::Vec3b>(xCoord2.x, xCoord2.y);
        glm::vec3 xAverage = { xPixel1.val[0] + xPixel2.val[0],xPixel1.val[1] + xPixel2.val[1] , xPixel1.val[2] + xPixel2.val[2] };
        xAverage /= 2.f;
        
        xImage.at<cv::Vec3b>(xCoord1.x, xCoord1.y).val[0] = xAverage.x;
        xImage.at<cv::Vec3b>(xCoord1.x, xCoord1.y).val[1] = xAverage.y;
        xImage.at<cv::Vec3b>(xCoord1.x, xCoord1.y).val[2] = xAverage.z;

        xImage.at<cv::Vec3b>(xCoord2.x, xCoord2.y).val[0] = xAverage.x;
        xImage.at<cv::Vec3b>(xCoord2.x, xCoord2.y).val[1] = xAverage.y;
        xImage.at<cv::Vec3b>(xCoord2.x, xCoord2.y).val[2] = xAverage.z;
    }

    void AverageFourPixels(cv::Mat& xImage, glm::vec2 xCoord1, glm::vec2 xCoord2, glm::vec2 xCoord3, glm::vec2 xCoord4) {
        cv::Vec3b xPixel1 = xImage.at<cv::Vec3b>(xCoord1.x, xCoord1.y);
        cv::Vec3b xPixel2 = xImage.at<cv::Vec3b>(xCoord2.x, xCoord2.y);
        cv::Vec3b xPixel3 = xImage.at<cv::Vec3b>(xCoord3.x, xCoord3.y);
        cv::Vec3b xPixel4 = xImage.at<cv::Vec3b>(xCoord4.x, xCoord4.y);
        glm::vec3 xAverage = { xPixel1.val[0] + xPixel2.val[0] + xPixel3.val[0] + xPixel4.val[0],xPixel1.val[1] + xPixel2.val[1] + xPixel3.val[1] + xPixel4.val[1] , xPixel1.val[2] + xPixel2.val[2] + xPixel3.val[2] + xPixel4.val[2] };
        xAverage /= 4.f;

        xImage.at<cv::Vec3b>(xCoord1.x, xCoord1.y).val[0] = xAverage.x;
        xImage.at<cv::Vec3b>(xCoord1.x, xCoord1.y).val[1] = xAverage.y;
        xImage.at<cv::Vec3b>(xCoord1.x, xCoord1.y).val[2] = xAverage.z;

        xImage.at<cv::Vec3b>(xCoord2.x, xCoord2.y).val[0] = xAverage.x;
        xImage.at<cv::Vec3b>(xCoord2.x, xCoord2.y).val[1] = xAverage.y;
        xImage.at<cv::Vec3b>(xCoord2.x, xCoord2.y).val[2] = xAverage.z;

        xImage.at<cv::Vec3b>(xCoord3.x, xCoord3.y).val[0] = xAverage.x;
        xImage.at<cv::Vec3b>(xCoord3.x, xCoord3.y).val[1] = xAverage.y;
        xImage.at<cv::Vec3b>(xCoord3.x, xCoord3.y).val[2] = xAverage.z;

        xImage.at<cv::Vec3b>(xCoord4.x, xCoord4.y).val[0] = xAverage.x;
        xImage.at<cv::Vec3b>(xCoord4.x, xCoord4.y).val[1] = xAverage.y;
        xImage.at<cv::Vec3b>(xCoord4.x, xCoord4.y).val[2] = xAverage.z;
    }

    void PreprocessHeightmap(cv::Mat& xHeightmap) {
        uint32_t rows = xHeightmap.rows;
        uint32_t cols = xHeightmap.cols;

        // Iterate over each TERRAIN_SIZE x TERRAIN_SIZE section
        for (uint32_t y = 0; y < rows; y += TERRAIN_SIZE) {
            for (uint32_t x = 0; x < cols; x += TERRAIN_SIZE) {

                if (y > 0 && y < rows) {
                    for (uint32_t i = 0; i < TERRAIN_SIZE; i++) {
                        AverageTwoPixels(xHeightmap, { x + i, y }, { x + i,y - 1 });
                    }
                }
                if (x > 0 && x < cols) {
                    for (uint32_t i = 0; i < TERRAIN_SIZE; i++) {
                        AverageTwoPixels(xHeightmap, { x, y + i }, { x - 1,y + i });
                    }
                }

                if (y > 0 && y < rows && x > 0 && x < cols)
                    AverageFourPixels(xHeightmap, { x,y }, { x - 1, y }, { x,y - 1 }, { x - 1,y - 1 });
                
            }
        }
    }

#define HEIGHTMAP_MESH_DENSITY 1

    Mesh* WriteMesh(cv::Mat& xImage) {
        uint32_t uWidth = xImage.cols;
        uint32_t uHeight = xImage.rows;

        Mesh* mesh = Mesh::Create();
        mesh->m_pxBufferLayout = new BufferLayout();
        glm::vec3 vertexScale = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec2 textureScale = glm::vec2(100, 100);
        mesh->m_uNumVerts = uWidth * uHeight * HEIGHTMAP_MESH_DENSITY * HEIGHTMAP_MESH_DENSITY;
        mesh->m_uNumIndices = ((uWidth * HEIGHTMAP_MESH_DENSITY) - 1) * ((uHeight * HEIGHTMAP_MESH_DENSITY) - 1) * 6;
        mesh->m_pxVertexPositions = new glm::highp_vec3[mesh->m_uNumVerts];
        mesh->m_pxUVs = new glm::vec2[mesh->m_uNumVerts];
        mesh->m_pxNormals = new glm::vec3[mesh->m_uNumVerts];
        mesh->m_pxTangents = new glm::vec3[mesh->m_uNumVerts];
        for (size_t i = 0; i < mesh->m_uNumVerts; i++)
        {
            mesh->m_pxNormals[i] = { 0,0,0 };
            mesh->m_pxTangents[i] = { 0,0,0 };
        }
        mesh->m_puIndices = new unsigned int[mesh->m_uNumIndices];



        for (uint32_t z = 0; z < uHeight * HEIGHTMAP_MESH_DENSITY; ++z) {
            for (uint32_t x = 0; x < uWidth * HEIGHTMAP_MESH_DENSITY; ++x) {
                glm::vec2 xUV = { (double)x / HEIGHTMAP_MESH_DENSITY , (double)z / HEIGHTMAP_MESH_DENSITY };
                uint32_t offset = (z * uWidth * HEIGHTMAP_MESH_DENSITY) + x;

                uint32_t x0 = std::floor(xUV.x);
                uint32_t x1 = std::min((uint32_t)std::ceil(xUV.x), uWidth-1);
                uint32_t y0 = std::floor(xUV.y);
                uint32_t y1 = std::min((uint32_t)std::ceil(xUV.y), uHeight -1);

                typedef cv::Vec<float, 3> type;

                type topLeft = xImage.at<type>(y0, x0);
                type topRight = xImage.at<type>(y0, x1);
                type bottomLeft = xImage.at<type>(y1, x0);
                type bottomRight = xImage.at<type>(y1, x1);

                double weightX = xUV.x - x0;
                double weightY = xUV.y - y0;


                double top = topRight.val[0] * weightX + topLeft.val[0] * (1.f - weightX);
                double bottom = bottomRight.val[0] * weightX + bottomLeft.val[0] * (1.f - weightX);

                double finalVal = bottom * weightY + top * (1.f - weightY);

                mesh->m_pxVertexPositions[offset] = glm::highp_vec3((double)x / HEIGHTMAP_MESH_DENSITY, finalVal * 100.l, (double)z / HEIGHTMAP_MESH_DENSITY) * vertexScale;
                glm::vec2 fUV = glm::vec2(x, z) / textureScale;
                mesh->m_pxUVs[offset] = fUV / (float)HEIGHTMAP_MESH_DENSITY;

            }
        }

        size_t i = 0;
        for (int z = 0; z < (uHeight * HEIGHTMAP_MESH_DENSITY) - 1; ++z) {
            for (int x = 0; x < (uWidth * HEIGHTMAP_MESH_DENSITY) - 1; ++x) {
                int a = (z * (uWidth * HEIGHTMAP_MESH_DENSITY)) + x;
                int b = (z * (uWidth * HEIGHTMAP_MESH_DENSITY)) + x + 1;
                int c = ((z + 1) * (uWidth * HEIGHTMAP_MESH_DENSITY)) + x + 1;
                int d = ((z + 1) * (uWidth * HEIGHTMAP_MESH_DENSITY)) + x;
                mesh->m_puIndices[i++] = a;
                mesh->m_puIndices[i++] = c;
                mesh->m_puIndices[i++] = b;
                mesh->m_puIndices[i++] = c;
                mesh->m_puIndices[i++] = a;
                mesh->m_puIndices[i++] = d;
            }
        }

        mesh->GenerateNormals();

        return mesh;

        std::string strName = "TerrainMesh";
        std::ofstream file(strName + ".obj");

        
        std::chrono::high_resolution_clock::time_point xNow = std::chrono::high_resolution_clock::now();

        std::stringstream strPositions;
        std::stringstream strUVs;
        std::stringstream strFaces;
        std::thread xPositionsThread([&strPositions, &mesh](void) {
            for (uint32_t i = 0; i < mesh->m_uNumVerts; i++) {
                glm::highp_vec3 pos = mesh->m_pxVertexPositions[i];
                strPositions << "v ";
                strPositions << pos[0] << " ";
                strPositions << pos[1] << " ";
                strPositions << pos[2] << '\n';
            }
            });
        std::thread xUVsThread([&strUVs, &mesh](void) {
            for (uint32_t i = 0; i < mesh->m_uNumVerts; i++) {
                glm::vec2 uv = mesh->m_pxUVs[i];
                strUVs << "vt ";
                strUVs << uv[0] << " ";
                strUVs << uv[1] << '\n';
            }
            });
        std::thread xFacesThread([&strFaces, &mesh](void) {
            for (uint32_t i = 0; i < mesh->m_uNumIndices; i += 3) {
                strFaces << "f ";
                strFaces << mesh->m_puIndices[i] + 1 << '/' << mesh->m_puIndices[i] + 1 << ' ';
                strFaces << mesh->m_puIndices[i + 1] + 1 << '/' << mesh->m_puIndices[i + 1] + 1 << ' ';
                strFaces << mesh->m_puIndices[i + 2] + 1 << '/' << mesh->m_puIndices[i + 2] + 1;
                strFaces << '\n';
            }
            });

        file << "o " << strName << '\n';
        xPositionsThread.join();
        xUVsThread.join();
        xFacesThread.join();
        file << strPositions.str();
        file << strUVs.str();
        file << strFaces.str();
        
        
        
        file.close();
        return mesh;

        uint32_t uNumSeconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - xNow).count();
        VCE_TRACE("Mesh export took {} seconds", uNumSeconds);
    }

    

    void GenerateHeightmapData() {
        cv::Mat xHeightmap = cv::imread(TEXTUREDIR + "Heightmaps/Test/heightmap.hdr", cv::IMREAD_ANYDEPTH);

        VCE_ASSERT(!xHeightmap.empty(), "Invalid image");

        uint32_t uImageWidth = xHeightmap.cols;
        uint32_t uImageHeight = xHeightmap.rows;

        

        VCE_ASSERT((uImageWidth * HEIGHTMAP_MESH_DENSITY) % TERRAIN_SIZE == 0, "Invalid terrain width");
        VCE_ASSERT((uImageHeight * HEIGHTMAP_MESH_DENSITY) % TERRAIN_SIZE == 0, "Invalid terrain height");

        uint32_t uNumSplitsX = uImageWidth / TERRAIN_SIZE;
        uint32_t uNumSplitsZ = uImageHeight / TERRAIN_SIZE;

        std::ofstream xAssetsOut("heightmap.vceassets");
        std::ofstream xSceneOut("heightmap.vcescene");

        Mesh* pxMesh = WriteMesh(xHeightmap);
        

        for (uint32_t z = 0; z < uNumSplitsZ; z++) {
            for (uint32_t x = 0; x < uNumSplitsX; x++) {
                Mesh* pxSubMesh = Mesh::Create();
                pxSubMesh->m_pxBufferLayout = new BufferLayout();
                pxSubMesh->m_uNumVerts = (TERRAIN_SIZE + 1) * (TERRAIN_SIZE + 1) * HEIGHTMAP_MESH_DENSITY * HEIGHTMAP_MESH_DENSITY;
                pxSubMesh->m_uNumIndices = (((TERRAIN_SIZE + 1) * HEIGHTMAP_MESH_DENSITY) - 1) * (((TERRAIN_SIZE + 1) * HEIGHTMAP_MESH_DENSITY) - 1) * 6;
                pxSubMesh->m_pxVertexPositions = new glm::highp_vec3[pxSubMesh->m_uNumVerts];
                pxSubMesh->m_pxUVs = new glm::vec2[pxSubMesh->m_uNumVerts];
                pxSubMesh->m_pxNormals = new glm::vec3[pxSubMesh->m_uNumVerts];
                pxSubMesh->m_pxTangents = new glm::vec3[pxSubMesh->m_uNumVerts];
                for (size_t i = 0; i < pxSubMesh->m_uNumVerts; i++)
                {
                    pxSubMesh->m_pxNormals[i] = { 0,0,0 };
                    pxSubMesh->m_pxTangents[i] = { 0,0,0 };
                }
                pxSubMesh->m_puIndices = new uint32_t[pxSubMesh->m_uNumIndices] {0};

                uint32_t uHeighestNewOffset = 0;
#ifdef VCE_DEBUG
                std::set<uint32_t> xFoundOldIndices;
                std::set<uint32_t> xFoundNewIndices;
                //std::unordered_set <glm::highp_vec3> xPositionsSet;
#endif
                std::array<uint32_t, TERRAIN_SIZE* HEIGHTMAP_MESH_DENSITY> xRightEdgeIndices;

                glm::highp_vec3 xOrigin = { x, 0, z };
                for (uint32_t subZ = 0; subZ < TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY; subZ++) {
                    for (uint32_t subX = 0; subX < TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY; subX++) {
                        uint32_t uNewOffset = (subZ * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY) + subX;

                        uint32_t uStartOfRow = (subZ * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY * uNumSplitsZ) + (z * uImageWidth * HEIGHTMAP_MESH_DENSITY * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY);
                        VCE_ASSERT(uStartOfRow < pxMesh->m_uNumVerts, "Start of row has gone past end of mesh");
                        uint32_t uIndexIntoRow = subX + x * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY;
                        VCE_ASSERT(uIndexIntoRow < HEIGHTMAP_MESH_DENSITY * uImageWidth, "Gone past end of row");
                        uint32_t uOldOffset = uStartOfRow + uIndexIntoRow;

                        VCE_ASSERT(uOldOffset < pxMesh->m_uNumVerts, "Incorrect index somewhere");

                        VCE_ASSERT(xFoundOldIndices.find(uOldOffset) == xFoundOldIndices.end(), "Duplicate old index");
                        VCE_ASSERT(xFoundNewIndices.find(uNewOffset) == xFoundNewIndices.end(), "Duplicate new index");
                        

                        pxSubMesh->m_pxVertexPositions[uNewOffset] = pxMesh->m_pxVertexPositions[uOldOffset];
                        pxSubMesh->m_pxUVs[uNewOffset] = pxMesh->m_pxUVs[uOldOffset];
                        pxSubMesh->m_pxNormals[uNewOffset] = pxMesh->m_pxNormals[uOldOffset];

                        if (subX == TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY - 1)
                            xRightEdgeIndices.at(subZ) = uNewOffset;


                        //VCE_ASSERT(xPositionsSet.find(pxSubMesh->m_pxVertexPositions[uNewOffset]) == xPositionsSet.end(), "Duplicate position");

#if 0
                        xOrigin.x = std::max(xOrigin.x, pxSubMesh->m_pxVertexPositions[uNewOffset].x);
                        xOrigin.y = std::max(xOrigin.y, pxSubMesh->m_pxVertexPositions[uNewOffset].y);
                        xOrigin.z = std::max(xOrigin.z, pxSubMesh->m_pxVertexPositions[uNewOffset].z);
#endif

#ifdef VCE_DEBUG
                        uHeighestNewOffset = std::max(uHeighestNewOffset, uNewOffset);
                        xFoundOldIndices.insert(uOldOffset);
                        xFoundNewIndices.insert(uNewOffset);
                        //xPositionsSet.insert(pxSubMesh->m_pxVertexPositions[uNewOffset]);
#endif
                    }
                }

#if 0
                for (uint32_t i = 0; i < pxSubMesh->m_uNumVerts; i++)
                    pxSubMesh->m_pxVertexPositions[i] += xOrigin;
#endif

                //VCE_ASSERT(uHeighestNewOffset == pxSubMesh->m_uNumVerts - 1, "Incorrect index somewhere");

                size_t indexIndex = 0;
                for (uint32_t indexZ = 0; indexZ < TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY - 1; indexZ++) {
                    for (uint32_t indexX = 0; indexX < TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY - 1; indexX++) {
                        uint32_t a = (indexZ * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY) + indexX;
                        uint32_t b = (indexZ * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY) + indexX + 1;
                        uint32_t c = ((indexZ + 1) * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY) + indexX + 1;
                        uint32_t d = ((indexZ + 1) * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY) + indexX;
                        pxSubMesh->m_puIndices[indexIndex++] = a;
                        pxSubMesh->m_puIndices[indexIndex++] = c;
                        pxSubMesh->m_puIndices[indexIndex++] = b;
                        pxSubMesh->m_puIndices[indexIndex++] = c;
                        pxSubMesh->m_puIndices[indexIndex++] = a;
                        pxSubMesh->m_puIndices[indexIndex++] = d;
                        VCE_ASSERT(indexIndex <= pxSubMesh->m_uNumIndices, "Index index too big");
                    }
                }

                if (x < uNumSplitsX - 1) {
                    uint32_t subX = TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY;
                    for (uint32_t subZ = 0; subZ < TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY; subZ++) {
                        uint32_t uNewOffset = ++uHeighestNewOffset;

                        VCE_ASSERT(uNewOffset < pxSubMesh->m_uNumVerts, "Offset too big for submesh");

                        uint32_t uStartOfRow = (subZ * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY * uNumSplitsZ) + (z * uImageWidth * HEIGHTMAP_MESH_DENSITY * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY);
                        VCE_ASSERT(uStartOfRow < pxMesh->m_uNumVerts, "Start of row has gone past end of mesh");
                        uint32_t uIndexIntoRow = subX + x * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY;
                        VCE_ASSERT(uIndexIntoRow < HEIGHTMAP_MESH_DENSITY * uImageWidth, "Gone past end of row");
                        uint32_t uOldOffset = uStartOfRow + uIndexIntoRow;

                        VCE_ASSERT(uOldOffset < pxMesh->m_uNumVerts, "Incorrect index somewhere");

                        VCE_ASSERT(xFoundOldIndices.find(uOldOffset) == xFoundOldIndices.end(), "Duplicate old index");
                        VCE_ASSERT(xFoundNewIndices.find(uNewOffset) == xFoundNewIndices.end(), "Duplicate new index");


                        pxSubMesh->m_pxVertexPositions[uNewOffset] = pxMesh->m_pxVertexPositions[uOldOffset];
                        pxSubMesh->m_pxUVs[uNewOffset] = pxMesh->m_pxUVs[uOldOffset];
                        pxSubMesh->m_pxNormals[uNewOffset] = pxMesh->m_pxNormals[uOldOffset];


                        //VCE_ASSERT(xPositionsSet.find(pxSubMesh->m_pxVertexPositions[uNewOffset]) == xPositionsSet.end(), "Duplicate position");

#if 0
                        xOrigin.x = std::max(xOrigin.x, pxSubMesh->m_pxVertexPositions[uNewOffset].x);
                        xOrigin.y = std::max(xOrigin.y, pxSubMesh->m_pxVertexPositions[uNewOffset].y);
                        xOrigin.z = std::max(xOrigin.z, pxSubMesh->m_pxVertexPositions[uNewOffset].z);
#endif

#ifdef VCE_DEBUG
                        uHeighestNewOffset = std::max(uHeighestNewOffset, uNewOffset);
                        xFoundOldIndices.insert(uOldOffset);
                        xFoundNewIndices.insert(uNewOffset);
                        //xPositionsSet.insert(pxSubMesh->m_pxVertexPositions[uNewOffset]);
#endif
                        
                    }

                    uHeighestNewOffset -= TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY - 1;

                    uint32_t indexX = TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY - 1;
                    for (uint32_t indexZ = 0; indexZ < TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY - 1; indexZ++) {
                        /*
                        uint32_t a = (indexZ * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY) + indexX;
                        uint32_t b = (indexZ * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY) + indexX + 1;
                        uint32_t c = ((indexZ + 1) * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY) + indexX + 1;
                        uint32_t d = ((indexZ + 1) * TERRAIN_SIZE * HEIGHTMAP_MESH_DENSITY) + indexX;
                        */
                        uint32_t a = xRightEdgeIndices.at(indexZ + 1);
                        uint32_t c = uHeighestNewOffset++;
                        uint32_t b = xRightEdgeIndices.at(indexZ);
                        uint32_t d = uHeighestNewOffset;

                        pxSubMesh->m_puIndices[indexIndex++] = a;
                        pxSubMesh->m_puIndices[indexIndex++] = c;
                        pxSubMesh->m_puIndices[indexIndex++] = b;
                        pxSubMesh->m_puIndices[indexIndex++] = c;
                        pxSubMesh->m_puIndices[indexIndex++] = a;
                        pxSubMesh->m_puIndices[indexIndex++] = d;
                        VCE_ASSERT(indexIndex <= pxSubMesh->m_uNumIndices, "Index index too big");
                    }
                }



                pxSubMesh->WriteToObj((MESHDIR + "Terrain/" + std::to_string(x) + "_" + std::to_string(z) + ".obj").c_str());

                

                GUID xAssetGUID;
                GUID xSceneGUID;
                xAssetsOut << "Mesh\n" << xAssetGUID.m_uGuid << '\n' << "Terrain/" << std::to_string(x) + "_" + std::to_string(z) + ".obj\n";
                xSceneOut << "Entity\n" << xSceneGUID.m_uGuid << '\n' << "0\n" << "Terrain" << std::to_string(x) + "_" + std::to_string(z) << '\n' << "TerrainComponent\n" << xAssetGUID.m_uGuid << "\n1538048126\n" << x << ' ' << z << "\nEndEntity\n";
            }
        }

#if 0
        GUID xAssetGUID;
        GUID xScene0GUID;
        GUID xScene1GUID;
        GUID xScene2GUID;
        xAssetsOut << "Mesh\n" << xAssetGUID.m_uGuid << '\n' << "Terrain/" << "Whole_Terrain" << ".obj\n";
        pxMesh->WriteToObj((MESHDIR + "Terrain/" + "Whole_Terrain" + ".obj").c_str());


        xSceneOut << "Entity\n" << xScene0GUID.m_uGuid << '\n' << "0\n" << "Terrain" << std::to_string(4) + "_" + std::to_string(0) << '\n' << "TerrainComponent\n" << xAssetGUID.m_uGuid << "\n1538048126\n" << 4 << ' ' << 0 << "\nEndEntity\n";
        xSceneOut << "Entity\n" << xScene1GUID.m_uGuid << '\n' << "0\n" << "Terrain" << std::to_string(0) + "_" + std::to_string(4) << '\n' << "TerrainComponent\n" << xAssetGUID.m_uGuid << "\n1538048126\n" << 0 << ' ' << 4 << "\nEndEntity\n";
        xSceneOut << "Entity\n" << xScene2GUID.m_uGuid << '\n' << "0\n" << "Terrain" << std::to_string(4) + "_" + std::to_string(4) << '\n' << "TerrainComponent\n" << xAssetGUID.m_uGuid << "\n1538048126\n" << 4 << ' ' << 4 << "\nEndEntity\n";
#endif

        xAssetsOut.close();
        xSceneOut.close();
        delete pxMesh;
    }
}