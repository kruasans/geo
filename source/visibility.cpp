#include "../header/visibility.h"
#include <cmath>
#include <iostream>
#include <filesystem>


// Функция для проверки взаимной видимости двух точек
// Функция для проверки взаимной видимости двух точек
bool isVisible(double lat1, double lon1, double additionalHeight1, double lat2, double lon2, double additionalHeight2) {
    int row1, col1, row2, col2;
    convertCoordinatesToIndices(lat1, lon1, row1, col1);
    convertCoordinatesToIndices(lat2, lon2, row2, col2);

    std::string file1 = getFileName(lat1, lon1);
    std::string file2 = getFileName(lat2, lon2);

    // Проверка на существование файлов для карт высот
    if (!std::filesystem::exists(file1) || !std::filesystem::exists(file2)) {
        std::cout << "Один из файлов карт высот не найден!" << std::endl;
        return false;
    }

    int h1 = getHeight(heightMaps[file1], row1, col1) + additionalHeight1;
    int h2 = getHeight(heightMaps[file2], row2, col2) + additionalHeight2;

    int dx = abs(col2 - col1);
    int dy = abs(row2 - row1);

    int sx = (col1 < col2) ? 1 : -1;
    int sy = (row1 < row2) ? 1 : -1;

    double dh = static_cast<double>(h2 - h1) / sqrt(dx * dx + dy * dy);

    int err = dx - dy;
    int x = col1;
    int y = row1;

    double currentHeight = h1;
    double dist = 0.0;

    // Инкременты для изменения координат
    double latStep = (lat2 - lat1) / (sqrt(dx * dx + dy * dy));
    double lonStep = (lon2 - lon1) / (sqrt(dx * dx + dy * dy));

    while (x != col2 || y != row2) {
        dist = sqrt((x - col1) * (x - col1) + (y - row1) * (y - row1));
        currentHeight = h1 + dist * dh;

        // Визуализация координат на каждом шаге
        double lat = lat1 + latStep * dist;
        double lon = lon1 + lonStep * dist;

        std::string currentFile = getFileName(lat, lon);

        // Проверка высоты на стыке карт
        if (heightMaps.find(currentFile) == heightMaps.end()) {
            std::cout << "Карта не найдена для координат: " << lat << ", " << lon << std::endl;
            return false;
        }

        // Проверка препятствий по высоте
        if (getHeight(heightMaps[currentFile], y, x) > currentHeight) {
            return false;  // Обнаружено препятствие
        }

        // Алгоритм Брезенхема для перемещения по линии
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }

    return true;
}

