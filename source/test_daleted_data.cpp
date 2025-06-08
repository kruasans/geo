#include "../header/test_deleted_data.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cmath>



const int SRTM_SIZE = 3601;
short height[SRTM_SIZE][SRTM_SIZE] = {0};

// Функция для чтения данных из HGT файла
void readHGTFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    unsigned char buffer[2];
    for (int i = 0; i < SRTM_SIZE; ++i) {
        for (int j = 0; j < SRTM_SIZE; ++j) {
            if (!file.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
                std::cerr << "Error reading file at row " << i << ", col " << j << std::endl;
                return;
            }
            height[i][j] = (buffer[0] << 8) | buffer[1];
        }
    }

    file.close();
}

// Функция для записи всех точек и их высоты в файл, исключая пропуски данных
void writeAllPointsToFile(const std::string& outputFilename) {
    std::ofstream outputFile(outputFilename);
    if (!outputFile) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        return;
    }

    for (int i = 0; i < SRTM_SIZE; ++i) {
        for (int j = 0; j < SRTM_SIZE; ++j) {
            if (height[i][j] != -32768) { // Пропускаем точки с пропусками данных
                outputFile << "Point (" << i << ", " << j << "): " << height[i][j] << " meters" << std::endl;
            }
        }
    }

    outputFile.close();
}

// Функция для записи пропусков данных в отдельный файл
void writeDataGapsToFile(const std::string& errorFilename) {
    std::ofstream errorFile(errorFilename);
    if (!errorFile) {
        std::cerr << "Error opening error file: " << errorFilename << std::endl;
        return;
    }

    bool hasDataGaps = false;
    for (int i = 0; i < SRTM_SIZE; ++i) {
        for (int j = 0; j < SRTM_SIZE; ++j) {
            if (height[i][j] == -32768) { // Значение -32768 обычно используется для обозначения пропусков данных
                errorFile << "Data gap at point (" << i << ", " << j << ")" << std::endl;
                hasDataGaps = true;
            }
        }
    }
    if (!hasDataGaps) {
        errorFile << "No data gaps found." << std::endl;
    }

    errorFile.close();
}