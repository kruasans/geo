#ifndef TEST_DELETED_DATA_H
#define TEST_DELETED_DATA_H

#include <fstream>

// Функция для чтения данных из HGT файла
void readHGTFile(const std::string& filename);

// Функция для записи всех точек и их высоты в файл, исключая пропуски данных
void writeAllPointsToFile(const std::string& outputFilename);

// Функция для записи пропусков данных в отдельный файл
void writeDataGapsToFile(const std::string& errorFilename);

#endif //TEST_DELETED_DATA_H
