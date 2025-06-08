#include "../header/coverage.h"
#include "../header/visibility.h"

#include <algorithm>  // Для std::remove
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <unordered_map>


// Глобальная переменная для хранения покрытия каждой вышки
std::unordered_map<std::string, TowerCoverage> towerCoverages;

// Функция для вычисления покрытия вышки и проверки видимости точек
void calculateCoverage(double towerLat, double towerLon, double areaSideKm, double stepKm, double& minLat,
                       double& maxLat, double& minLon, double& maxLon, double& stepDegLat, double& stepDegLon)
{
    const double kmPerDegreeLat = 111.0;
    const double kmPerDegreeLon = 111.0 * cos(towerLat * M_PI / 180.0);
    const double halfSideDegLat = (areaSideKm / 2) / kmPerDegreeLat;
    const double halfSideDegLon = (areaSideKm / 2) / kmPerDegreeLon;

    minLat = towerLat - halfSideDegLat;
    maxLat = towerLat + halfSideDegLat;
    minLon = towerLon - halfSideDegLon;
    maxLon = towerLon + halfSideDegLon;

    stepDegLat = stepKm / kmPerDegreeLat;
    stepDegLon = stepKm / kmPerDegreeLon;
}

void visiblePoints(double towerLat, double towerLon, double areaSideKm, double stepKm, const std::string& towerId)
{
    std::cout << "Обработка вышки: " << towerId << " с координатами (" << towerLat << ", " << towerLon << ")\n";
    std::cout << "Проверка видимости точек в области " << areaSideKm << " км x " << areaSideKm << " км...\n";
    std::vector<Point> visiblePoints;

    double minLat, maxLat, minLon, maxLon, stepDegLat, stepDegLon;

    calculateCoverage(towerLat, towerLon, areaSideKm, stepKm, minLat, maxLat, minLon, maxLon, stepDegLat, stepDegLon);

    for (double lat = minLat; lat <= maxLat; lat += stepDegLat)
    {
        for (double lon = minLon; lon <= maxLon; lon += stepDegLon)
        {
            if (isVisible(towerLat, towerLon, 30, lat, lon, 0))
            {
                visiblePoints.push_back({lat, lon});
            }
        }
    }

    std::cout << "Найдено " << visiblePoints.size() << " видимых точек для вышки: " << towerId << "\n";
    towerCoverages[towerId] = {towerLat, towerLon, visiblePoints};
}

// Функция для сохранения координат объекта в CSV
void saveCoordinatesToCSV(double minLat, double maxLat, double minLon, double maxLon, double stepDegLat,
                          double stepDegLon)
{
    std::ofstream file("../data/coverage/object_coordinates.csv");
    if (!file.is_open())
    {
        std::cerr << "Не удалось открыть файл для записи!" << std::endl;
        return;
    }

    // Записываем заголовок CSV
    file << "Latitude,Longitude" << std::endl;

    // Перебираем все точки объекта и записываем их в CSV
    for (double lat = minLat; lat <= maxLat; lat += stepDegLat)
    {
        for (double lon = minLon; lon <= maxLon; lon += stepDegLon)
        {
            file << lat << "," << lon << std::endl; // Записываем координаты
        }
    }

    file.close();
}

// Функция для проверки, виден ли объект с вышки
bool isObjectVisibleFromTower(const std::string& towerId, double minLat,
                              double maxLat, double minLon, double maxLon, double stepDegLat, double stepDegLon)
{
    saveCoordinatesToCSV(minLat, maxLat, minLon, maxLon, stepDegLat, stepDegLon);
    // Считаем все точки, которые составляют объект
    int totalPoints = 0;
    int coveredPoints = 0;

    for (double lat = minLat; lat <= maxLat; lat += stepDegLat)
    {
        for (double lon = minLon; lon <= maxLon; lon += stepDegLon)
        {
            totalPoints++; // Считаем все точки объекта

            // Проверяем, покрывает ли точка хотя бы одна из точек видимости вышки
            for (const auto& point : towerCoverages[towerId].visiblePoints)
            {
                if (point.lat >= minLat && point.lat <= maxLat && point.lon >= minLon && point.lon <= maxLon)
                {
                    coveredPoints++; // Увеличиваем количество покрытых точек
                    break; // Если точка уже покрыта, выходим из цикла
                }
            }
        }
    }

    // Проверяем, что хотя бы 80% точек объекта покрыты
    if ((coveredPoints / static_cast<double>(totalPoints)) >= 0.1)
    {
        return true; // Объект покрыт на 80% и более
    }

    return false; // Объект не покрыт на 80%
}



bool isIntersecting(const TowerCoverage& towerCoverage1, const TowerCoverage& towerCoverage2)
{
    // Проходим по всем точкам первой вышки
    for (const auto& point1 : towerCoverage1.visiblePoints)
    {
        // Проверяем, есть ли эта точка в видимых точках второй вышки
        for (const auto& point2 : towerCoverage2.visiblePoints)
        {
            if (point1.lat == point2.lat && point1.lon == point2.lon)
            {
                // Вышка покрывает объект
                return true;
            }
        }
    }
    // Если общих точек нет, значит покрытия не пересекаются
    return false;
}

void selectTowersForMaxCoverage(double targetLat, double targetLon)
{
    std::vector<std::string> selectedTowers;
    std::vector<std::string> candidates;
    double minLat, maxLat, minLon, maxLon, stepDegLat, stepDegLon;
    double areaSideKm = 0.1; // Размер объекта 100x100 м = 0.1 км
    double stepKm = 0.03; // Шаг для точек видимости

    // Получаем координаты объекта
    calculateCoverage(targetLat, targetLon, areaSideKm, stepKm, minLat, maxLat, minLon, maxLon, stepDegLat, stepDegLon);

    // Шаг 1: Ищем все вышки, которые могут покрывать объект
    for (const auto& tower : towerCoverages)
    {
        const auto& towerId = tower.first;
        const auto& coverage = tower.second;

        // Проверка видимости объекта с этой вышки
        if (isObjectVisibleFromTower(towerId, minLat, maxLat, minLon, maxLon, stepDegLat,
                                     stepDegLon))
        {
            candidates.push_back(towerId);
            std::cout << "Вышка " << towerId << " покрывает объект" << std::endl;
            // Если вышка видит объект, добавляем её в кандидаты
        }
    }

    // while (!candidates.empty())
    // {
    //     std::string bestTowerId;
    //     size_t maxCoverage = 0;
    //
    //     // Шаг 2: Выбираем вышку с максимальным количеством видимых точек
    //     for (const auto& towerId : candidates)
    //     {
    //         size_t coverageSize = towerCoverages[towerId].visiblePoints.size();
    //
    //         // Выбираем вышку с максимальным количеством видимых точек
    //         if (coverageSize > maxCoverage)
    //         {
    //             maxCoverage = coverageSize;
    //             bestTowerId = towerId;
    //         }
    //     }
    //
    //     // Шаг 3: Если мы нашли лучшую вышку, добавляем её в выбранные
    //     if (!bestTowerId.empty())
    //     {
    //         selectedTowers.push_back(bestTowerId);
    //         candidates.erase(std::remove(candidates.begin(), candidates.end(), bestTowerId), candidates.end());
    //     }
    //
    //     // Шаг 4: Проверка новых точек
    //     bool hasNewPoints = true;
    //     while (hasNewPoints && !candidates.empty())
    //     {
    //         size_t additionalCoverage = 0;
    //         size_t currentCoverage = towerCoverages[bestTowerId].visiblePoints.size();
    //
    //         // Ищем следующую вышку и проверяем, сколько новых точек она добавит
    //         std::string nextBestTowerId;
    //         size_t maxAdditionalCoverage = 0;
    //
    //         for (const auto& towerId : candidates)
    //         {
    //             size_t newCoverage = 0;
    //             for (const auto& point : towerCoverages[towerId].visiblePoints)
    //             {
    //                 bool isCovered = false;
    //                 for (const auto& selectedTower : selectedTowers)
    //                 {
    //                     if (isPointCoveredByTower(point, selectedTower))
    //                     {
    //                         isCovered = true;
    //                         break;
    //                     }
    //                 }
    //                 if (!isCovered)
    //                 {
    //                     newCoverage++;  // Если точка не покрыта, считаем её новым покрытием
    //                 }
    //             }
    //
    //             // Если новая вышка добавляет больше покрытия, сохраняем её
    //             if (newCoverage > maxAdditionalCoverage)
    //             {
    //                 maxAdditionalCoverage = newCoverage;
    //                 nextBestTowerId = towerId;
    //             }
    //         }
    //
    //         // Если новая вышка добавляет меньше 40% от покрытия текущей вышки, выбираем следующую
    //         if (maxAdditionalCoverage >= (0.8 * maxCoverage))
    //         {
    //             selectedTowers.push_back(nextBestTowerId);
    //             candidates.erase(std::remove(candidates.begin(), candidates.end(), nextBestTowerId), candidates.end());
    //             hasNewPoints = true;
    //         }
    //         else
    //         {
    //             hasNewPoints = false;
    //         }
    //     }
    //
    //     // Шаг 5: Убираем из кандидатов те вышки, которые не дают нового покрытия
    //     std::vector<std::string> towersToRemove;
    //     for (const auto& towerId : candidates)
    //     {
    //         bool hasNewCoverage = false;
    //         for (const auto& selectedTower : selectedTowers)
    //         {
    //             if (isIntersecting(towerCoverages[selectedTower], towerCoverages[towerId]))
    //             {
    //                 hasNewCoverage = true;
    //                 break;
    //             }
    //         }
    //         if (!hasNewCoverage)
    //         {
    //             towersToRemove.push_back(towerId); // Добавляем вышку в список для удаления
    //         }
    //     }
    //
    //     // Удаляем вышки, которые не дают нового покрытия
    //     for (const auto& towerId : towersToRemove)
    //     {
    //         candidates.erase(std::remove(candidates.begin(), candidates.end(), towerId), candidates.end());
    //     }
    // }

    // Выводим выбранные вышки
    std::cout << "Выбранные вышки: \n";
    for (const auto& towerId : candidates)
    {
        std::cout << towerId << std::endl;
    }
}


// Функция для чтения данных о вышках из файла
// void readTowersFromFile(const std::string& filename)
// {
//
// }

void writeCoverageToIndividualFiles()
{
    std::cout << "Запись данных о покрытии в файлы...\n";

    for (const auto& pair : towerCoverages)
    {
        const auto& coverage = pair.second;
        exportPointsToCSV(pair.first);
        // Формируем имя файла на основе ID вышки
        std::string filename = pair.first + "_coverage.txt";
        std::ofstream file(filename);

        if (!file)
        {
            std::cout << "Ошибка открытия файла для записи: " << filename << std::endl;
            continue;
        }
        // Записываем информацию о покрытии в файл
        file << "Tower: " << pair.first << " (" << coverage.towerLat << ", " << coverage.towerLon << ")\n";
        for (const auto& point : coverage.visiblePoints)
        {
            file << "  Lat: " << point.lat << ", Lon: " << point.lon << "\n";
        }

        std::cout << "Данные для " << pair.first << " записаны в файл: " << filename << "\n";
        file.close();
    }
}

void exportPointsToCSV(const std::string& towerId)
{
    std::string filename = "../data/coverage/" + towerId + "_coverage.csv";

    std::ofstream file(filename);

    if (!file)
    {
        std::cout << "Ошибка открытия файла для записи: " << filename << std::endl;
        return;
    }

    file << "Latitude,Longitude\n"; // Заголовки для координат

    for (const auto& point : towerCoverages[towerId].visiblePoints)
    {
        file << point.lat << "," << point.lon << "\n"; // Сохраняем координаты
    }

    file.close();
    std::cout << "Точки успешно экспортированы в файл: " << filename << std::endl;
}
