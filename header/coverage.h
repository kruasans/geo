#ifndef COVERAGE_H
#define COVERAGE_H

#include <vector>
#include <string>

struct Object
{
    double lat;
    double lon;
};

// Структура для хранения точки
struct Point {
    double lat;  // широта
    double lon;  // долгота
};

// Структура для хранения информации о покрытии для каждой вышки
struct TowerCoverage {
    double towerLat;
    double towerLon;
    std::vector<Point> visiblePoints;
};


void calculateCoverage(double towerLat, double towerLon, double areaSideKm, double stepKm,
                       double& minLat, double& maxLat, double& minLon, double& maxLon,
                       double& stepDegLat, double& stepDegLon);

void visiblePoints(double towerLat, double towerLon, double areaSideKm, double stepKm, const std::string& towerId);

bool isObjectVisibleFromTower(const std::string& towerId, double minLat, double maxLat,
                              double minLon, double maxLon, double stepDegLat, double stepDegLon);

bool isPointCoveredByTower(const Point& point, const std::string& towerId);

bool isIntersecting(const TowerCoverage& towerCoverage1, const TowerCoverage& towerCoverage2);

void selectTowersForMaxCoverage(double targetLat, double targetLon);

void readTowersFromFile(const std::string& filename);

void writeCoverageToIndividualFiles();

void exportPointsToCSV(const std::string& towerId);

#endif // COVERAGE_H