#ifndef HEIGHT_MAP_H
#define HEIGHT_MAP_H

// #include <cstdint>
#include <unordered_map>
#include <string>

const int SRTM_SIZE = 3601;

// Структура для хранения загруженных файлов
struct HeightMap {
    short height[SRTM_SIZE][SRTM_SIZE];
    bool loaded;
};

// Объявление глобальной переменной для хранения загруженных файлов высот
extern std::unordered_map<std::string, HeightMap> heightMaps;

// Функция для загрузки карты высот из файла
bool loadHeightMap(const std::string& filename, HeightMap& heightMap);

// Функция для преобразования координат в индексы массива
void convertCoordinatesToIndices(double latitude, double longitude, int &row, int &col);

// Функция для получения высоты по индексам
short getHeight(const HeightMap& heightMap, int row, int col);

// Функция для загрузки всех необходимых файлов высот
bool loadHeightMaps(double lat1, double lon1, double lat2, double lon2);

// Функция для получения имени файла по координатам
std::string getFileName(double latitude, double longitude);

#endif // HEIGHT_MAP_H
