#include "vcepch.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include "VeryCoolEngine/core.h"
#include "VeryCoolEngine/Components/TerrainComponent.h"
#include "VeryCoolEngine/PlatformTypes.h"


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

    void GenerateHeightmapData() {
        // Read the large source image
        cv::Mat xHeightmap = cv::imread("C:\\dev\\VeryCoolEngine\\Assets\\Textures\\Heightmaps\\Test\\heightmap.png");

        if (xHeightmap.empty()) {
            std::cerr << "Invalid image" << std::endl;
            return;
        }

        PreprocessHeightmap(xHeightmap);

        // Save the preprocessed heightmap as a PNG file
        cv::imwrite("C:\\dev\\VeryCoolEngine\\Assets\\Textures\\Heightmaps\\Test\\preprocessed_heightmap.png", xHeightmap);

        uint32_t uImageWidth = xHeightmap.cols;
        uint32_t uImageHeight = xHeightmap.rows;

        if (uImageWidth % TERRAIN_SIZE != 0 || uImageHeight % TERRAIN_SIZE != 0) {
            std::cerr << "Heightmap width or height is not multiple of TERRAIN_SIZE" << std::endl;
            return;
        }

        uint32_t uNumSplitsX = uImageWidth / TERRAIN_SIZE;
        uint32_t uNumSplitsY = uImageHeight / TERRAIN_SIZE;

        std::ofstream xAssetsOut("C:\\dev\\VeryCoolEngine\\Game\\heightmap.vceassets");
        std::ofstream xSceneOut("C:\\dev\\VeryCoolEngine\\Game\\heightmap.vcescene");

        for (uint32_t x = 0; x < uNumSplitsX; x++) {
            for (uint32_t y = 0; y < uNumSplitsY; y++) {
                std::string strOut = "C:\\dev\\VeryCoolEngine\\Assets\\Textures\\Heightmaps\\Test\\" + std::to_string(x) + "_" + std::to_string(y) + ".png";
                cv::Rect xRect(x * TERRAIN_SIZE, y * TERRAIN_SIZE, TERRAIN_SIZE, TERRAIN_SIZE);
                cv::Mat xImgOut = xHeightmap(xRect).clone(); // Clone to avoid modifying original data
                cv::Mat xImgOutFlipped;

                // Save the modified tile
                cv::flip(xImgOut, xImgOutFlipped, 0);
                cv::imwrite(strOut, xImgOutFlipped);

                // Output to asset and scene files
                GUID xAssetGUID;
                GUID xSceneGUID;
                xAssetsOut << "Texture2D\n" << xAssetGUID.m_uGuid << '\n' << "0\n" << "Heightmaps/Test/" << std::to_string(x) + "_" + std::to_string(y) + ".png\n";
                xSceneOut << "Entity\n" << xSceneGUID.m_uGuid << '\n' << "0\n" << "Terrain" << std::to_string(x) + "_" + std::to_string(y) << '\n' << "TerrainComponent\n" << xAssetGUID.m_uGuid << "\n1538048126\n" << x << ' ' << y << "\nEndEntity\n";
            }
        }

        xAssetsOut.close();
        xSceneOut.close();
    }
}