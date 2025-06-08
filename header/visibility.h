#ifndef VISIBILITY_H
#define VISIBILITY_H

#include "height_map.h"

// Функция для проверки взаимной видимости двух точек
bool isVisible(double lat1, double lon1, double additionalHeight1, double lat2, double lon2, double additionalHeight2);

#endif // VISIBILITY_H
