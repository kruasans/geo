#include "../header/config.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Default configuration values
double towerAreaSideKm = 12.0;
double towerStepKm = 0.01;         // Step for tower coverage points
double objectStepKm = 0.01;        // Step for object area grid
double areaSideKm = 0.1;           // Size of object area (km)
double newCoverageFraction = 0.6;  // Fraction of new points required from tower

bool loadConfig(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cout << "Config file not found: " << filename << ". Using default values." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Skip empty lines or comments
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '='))
        {
            std::string valueStr;
            if (std::getline(iss, valueStr))
            {
                double value = std::stod(valueStr);
                if (key == "towerAreaSideKm")
                    towerAreaSideKm = value;
                else if (key == "towerStepKm")
                    towerStepKm = value;
                else if (key == "objectStepKm")
                    objectStepKm = value;
                else if (key == "areaSideKm")
                    areaSideKm = value;
                else if (key == "newCoveragePercent")
                    newCoverageFraction = value / 100.0; // convert percent to fraction
            }
        }
    }
    return true;
}