//
// Created by kruasan on 13.06.2025.
//

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

extern double towerAreaSideKm;
extern double towerStepKm;      // Step between points for tower coverage
extern double objectStepKm;     // Step between points for object area
extern double areaSideKm;       // Size of object square side in km
extern double newCoverageFraction; // Required fraction of new points from tower coverage

bool loadConfig(const std::string& filename = "../data/config.txt");

#endif //CONFIG_H
