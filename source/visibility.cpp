#include "../header/visibility.h"
#include <cmath>
#include <iostream>
#include <filesystem>


// Функция для проверки взаимной видимости двух точек
bool isVisible(double lat1, double lon1, double additionalHeight1, double lat2,
               double lon2, double additionalHeight2) {
    int row1, col1, row2, col2;
    convertCoordinatesToIndices(lat1, lon1, row1, col1);
    convertCoordinatesToIndices(lat2, lon2, row2, col2);

    std::string file1 = getFileName(lat1, lon1);
    std::string file2 = getFileName(lat2, lon2);

    // Проверяем существование и при необходимости загружаем карты высот
    if (!std::filesystem::exists(file1) || !std::filesystem::exists(file2)) {
        std::cout << "Один из файлов карт высот не найден!" << std::endl;
        return false;
    }

    if (heightMaps.find(file1) == heightMaps.end()) {
        loadHeightMap(file1, heightMaps[file1]);
    }
    if (heightMaps.find(file2) == heightMaps.end()) {
        loadHeightMap(file2, heightMaps[file2]);
    }

    int h1 = getHeight(heightMaps[file1], row1, col1) + additionalHeight1;
    int h2 = getHeight(heightMaps[file2], row2, col2) + additionalHeight2;

    double distLat = lat2 - lat1;
    double distLon = lon2 - lon1;
    int steps = static_cast<int>(std::sqrt(distLat * distLat + distLon * distLon) *
                                 (SRTM_SIZE - 1));
    if (steps == 0) {
        return true;  // Точки совпадают
    }

    double latStep = distLat / steps;
    double lonStep = distLon / steps;
    double dh = static_cast<double>(h2 - h1) / steps;

    double lat = lat1;
    double lon = lon1;
    double currentHeight = h1;

    for (int i = 0; i <= steps; ++i) {
        std::string currentFile = getFileName(lat, lon);
        if (heightMaps.find(currentFile) == heightMaps.end()) {
            if (!std::filesystem::exists(currentFile)) {
                std::cout << "Карта не найдена для координат: " << lat << ", "
                          << lon << std::endl;
                return false;
            }
            loadHeightMap(currentFile, heightMaps[currentFile]);
        }

        int row, col;
        convertCoordinatesToIndices(lat, lon, row, col);

        if (getHeight(heightMaps[currentFile], row, col) > currentHeight) {
            return false;  // Обнаружено препятствие
        }

        lat += latStep;
        lon += lonStep;
        currentHeight += dh;
    }

    return true;
}

