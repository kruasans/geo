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
    double stepKm = 0.01; // Шаг для точек видимости

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

    if (candidates.empty())
    {
        std::cout << "Нет вышек, покрывающих объект" << std::endl;
        return;
    }

    // Шаг 2: выбираем вышку с максимальным покрытием
    std::string bestTower;
    size_t maxPoints = 0;
    for (const auto& id : candidates)
    {
        size_t size = towerCoverages[id].visiblePoints.size();
        if (size > maxPoints)
        {
            maxPoints = size;
            bestTower = id;
        }
    }

    selectedTowers.push_back(bestTower);

    std::vector<Point> coveredPoints = towerCoverages[bestTower].visiblePoints;
    candidates.erase(std::remove(candidates.begin(), candidates.end(), bestTower), candidates.end());

    bool added = true;
    while (added && !candidates.empty())
    {
        added = false;
        std::string nextTower;
        size_t maxNewCoverage = 0;

        for (const auto& candidate : candidates)
        {
            bool intersects = false;
            for (const auto& selected : selectedTowers)
            {
                if (isIntersecting(towerCoverages[selected], towerCoverages[candidate]))
                {
                    intersects = true;
                    break;
                }
            }
            if (!intersects)
                continue;

            size_t newCoverage = 0;
            for (const auto& point : towerCoverages[candidate].visiblePoints)
            {
                bool alreadyCovered = false;
                for (const auto& cp : coveredPoints)
                {
                    if (std::abs(cp.lat - point.lat) < 1e-6 && std::abs(cp.lon - point.lon) < 1e-6)
                    {
                        alreadyCovered = true;
                        break;
                    }
                }
                if (!alreadyCovered)
                    newCoverage++;
            }

            if (newCoverage > maxNewCoverage)
            {
                maxNewCoverage = newCoverage;
                nextTower = candidate;
            }
        }

        if (maxNewCoverage > 0)
        {
            selectedTowers.push_back(nextTower);
            coveredPoints.insert(coveredPoints.end(), towerCoverages[nextTower].visiblePoints.begin(),
                                 towerCoverages[nextTower].visiblePoints.end());
            candidates.erase(std::remove(candidates.begin(), candidates.end(), nextTower), candidates.end());
            added = true;
        }
    }

    std::cout << "Выбранные вышки: \n";
    for (const auto& id : selectedTowers)
    {
        std::cout << id << std::endl;
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
