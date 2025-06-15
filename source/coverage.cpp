#include "../header/coverage.h"
#include "../header/visibility.h"
#include "../header/config.h"

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

void visiblePoints(double towerLat, double towerLon, double towerHeight, double areaSideKm, double stepKm, const std::string& towerId)
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
            if (isVisible(towerLat, towerLon, towerHeight, lat, lon, 0))
            {
                visiblePoints.push_back({lat, lon});
            }
        }
    }

    std::cout << "Найдено " << visiblePoints.size() << " видимых точек для вышки: " << towerId << "\n";
    towerCoverages[towerId] = {towerLat, towerLon, towerHeight, visiblePoints};
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
    if ((coveredPoints / static_cast<double>(totalPoints)) >= 0.8)
    {
        return true;
    }

    return false;
}

bool isPointCoveredByTower(const Point& point, const std::string& towerId)
{
    const double eps = 1e-6;
    for (const auto& p : towerCoverages[towerId].visiblePoints)
    {
        if (std::abs(p.lat - point.lat) < eps && std::abs(p.lon - point.lon) < eps)
        {
            return true;
        }
    }
    return false;
}

size_t intersectionSize(const TowerCoverage& towerCoverage1, const TowerCoverage& towerCoverage2)
{
    size_t count = 0;
    for (const auto& p1 : towerCoverage1.visiblePoints)
    {
        for (const auto& p2 : towerCoverage2.visiblePoints)
        {
            if (std::abs(p1.lat - p2.lat) < 1e-6 && std::abs(p1.lon - p2.lon) < 1e-6)
            {
                ++count;
            }
        }
    }
    return count;
}

void selectTowersForMaxCoverage(double targetLat, double targetLon)
{
    std::vector<std::string> candidates;
    std::vector<std::string> selectedTowers;
    double minLat, maxLat, minLon, maxLon, stepDegLat, stepDegLon;
    double areaSide = areaSideKm;  // Размер объекта 100x100 м = 0.1 км
    double stepKm = objectStepKm;     // Шаг для точек видимости

    // Получаем координаты объекта
    calculateCoverage(targetLat, targetLon, areaSide, stepKm, minLat, maxLat, minLon, maxLon,
                      stepDegLat, stepDegLon);

    // Собираем вышки, которые покрывают объект хотя бы на 80%
    for (const auto& tower : towerCoverages)
    {
        const auto& towerId = tower.first;
        if (isObjectVisibleFromTower(towerId, minLat, maxLat, minLon, maxLon, stepDegLat, stepDegLon))
        {
            candidates.push_back(towerId);
            std::cout << "Вышка " << towerId << " покрывает объект" << std::endl;
        }
    }

    if (candidates.empty())
    {
        std::cout << "Нет вышек, покрывающих объект" << std::endl;
        return;
    }

    if (candidates.size() < 2)
    {
        std::cout << "Недостаточно вышек для выбора" << std::endl;
        return;
    }

    // constexpr double MIN_ANGLE_DIFF = M_PI / 2; // 90 градусов

    size_t bestIntersection = 0;
    std::pair<std::string, std::string> bestPair;

    for (size_t i = 0; i < candidates.size(); ++i)
    {
        for (size_t j = i + 1; j < candidates.size(); ++j)
        {
            const std::string& id1 = candidates[i];
            const std::string& id2 = candidates[j];

            // // Угол между вышками относительно объекта
            // double ang1 = atan2(towerCoverages[id1].towerLat - targetLat,
            //                     towerCoverages[id1].towerLon - targetLon);
            // double ang2 = atan2(towerCoverages[id2].towerLat - targetLat,
            //                     towerCoverages[id2].towerLon - targetLon);
            // double diff = std::fabs(ang1 - ang2);
            // if (diff > M_PI)
            //     diff = 2 * M_PI - diff;
            // if (diff < MIN_ANGLE_DIFF)
            //     continue; // расположены слишком близко друг к другу

            size_t inter = intersectionSize(towerCoverages[id1], towerCoverages[id2]);
            if (inter > bestIntersection)
            {
                bestIntersection = inter;
                bestPair = {id1, id2};
            }
        }
    }

    if (bestIntersection == 0)
    {
        std::cout << "Не найдено подходящей пары вышек" << std::endl;
        return;
    }

    std::cout << "Выбранные вышки:" << std::endl;
    std::cout << bestPair.first << std::endl;
    std::cout << bestPair.second << std::endl;
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
