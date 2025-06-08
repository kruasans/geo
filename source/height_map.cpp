#include "../header/height_map.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <sstream>

// Определение глобальной переменной для хранения загруженных файлов высот
std::unordered_map<std::string, HeightMap> heightMaps;

// Функция для загрузки карты высот из файла
bool loadHeightMap(const std::string& filename, HeightMap& heightMap) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        std::cout << "Ошибка открытия файла: " << filename << std::endl;
        return false;
    }

    unsigned char buffer[2];
    for (int i = 0; i < SRTM_SIZE; ++i) {
        for (int j = 0; j < SRTM_SIZE; ++j) {
            if (!file.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
                std::cout << "Ошибка чтения файла: " << filename << std::endl;
                return false;
            }
            heightMap.height[i][j] = (buffer[0] << 8) | buffer[1];
        }
    }
    heightMap.loaded = true;
    return true;
}

// Функция для преобразования координат в индексы массива
void convertCoordinatesToIndices(double latitude, double longitude, int &row, int &col) {
    row = static_cast<int>(std::round((latitude - std::floor(latitude)) * (SRTM_SIZE - 1)));
    col = static_cast<int>(std::round((longitude - std::floor(longitude)) * (SRTM_SIZE - 1)));
    row = SRTM_SIZE - 1 - row;
}

// Функция для получения высоты по индексам
short getHeight(const HeightMap& heightMap, int row, int col) {
    return heightMap.height[row][col];
}

// Функция для загрузки всех необходимых файлов высот
bool loadHeightMaps(double lat1, double lon1, double lat2, double lon2) {
    std::string file1 = getFileName(lat1, lon1);
    std::string file2 = getFileName(lat2, lon2);

    if (heightMaps.find(file1) == heightMaps.end()) {
        heightMaps[file1] = HeightMap();
        if (!loadHeightMap(file1, heightMaps[file1])) {
            return false;
        }
    }

    if (file1 != file2 && heightMaps.find(file2) == heightMaps.end()) {
        heightMaps[file2] = HeightMap();
        if (!loadHeightMap(file2, heightMaps[file2])) {
            return false;
        }
    }

    return true;
}

// Функция для получения имени файла по координатам
std::string getFileName(double latitude, double longitude) {
    std::ostringstream oss;
    oss << "../data/N" << std::abs(static_cast<int>(latitude));
    if (longitude < 100) {
        oss << "E0" << std::abs(static_cast<int>(longitude));
    } else {
        oss << "E" << std::abs(static_cast<int>(longitude));
    }
    oss << ".hgt";
    return oss.str();
}
