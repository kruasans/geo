#include <iostream>
#include <limits>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

#include "../header/coverage.h"
#include "../header/height_map.h"
#include "../header/visibility.h"

void showMenu()
{
    std::cout << "\nВыберите действие:\n"
        << "1. Проверка видимости между точками\n"
        << "2. Выбрать вышки для покрытия объекта\n"
        << "3. Выйти\n"
        << "Ваш выбор: ";
}

void clearInput()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main()
{
    bool running = true;
    double objectHeight = 0.0;

    while (running)
    {
        showMenu();

        int choice;
        std::cin >> choice;
        if (std::cin.fail())
        {
            std::cout << "❌ Ошибка ввода.\n";
            clearInput();
            continue;
        }

        switch (choice)
        {
        case 1:
            {
                double lat1, lon1, lat2, lon2, h1, h2;
                std::cout << "Введите координаты первой точки (lat lon): ";
                std::cin >> lat1 >> lon1;
                std::cout << "Введите высоту над уровнем земли для первой точки (м): ";
                std::cin >> h1;

                std::cout << "Введите координаты второй точки (lat lon): ";
                std::cin >> lat2 >> lon2;
                std::cout << "Введите высоту над уровнем земли для второй точки (м): ";
                std::cin >> h2;



                if (isVisible(lat1, lon1, h1, lat2, lon2, h2))
                {
                    std::cout << "✅ Точки видны друг другу.\n";
                }
                else
                {
                    std::cout << "❌ Точки не видны друг другу.\n";
                }
                break;
            }

        case 2:
            {
                std::string filename = "../data/towers.txt";
                std::ifstream file(filename);
                std::string towerId;
                double lat, lon;
                double objectLat, objectLon;

                if (!file)
                {
                    std::cout << "Ошибка открытия файла: " << filename << std::endl;
                    break;
                }

                std::cout << "Чтение данных о вышках из файла: " << filename << "\n";

                // Чтение координат объекта из первой строки
                if (file >> objectLat >> objectLon)
                {
                    std::cout << "Координаты объекта: (" << objectLat << ", " << objectLon << ")\n";
                }
                else
                {
                    std::cout << "Ошибка чтения координат объекта из первой строки!" << std::endl;
                    file.close();
                    break;
                }

                std::string firstLine;
                std::getline(file, firstLine);

                while (file >> towerId >> lat >> lon)
                {
                    visiblePoints(lat, lon, 12.0, 0.01, towerId); // 12 км квадрат, шаг 30 метров
                }

                file.close();

                selectTowersForMaxCoverage(objectLat, objectLon);

                // Запись данных о покрытии в файл
                writeCoverageToIndividualFiles();

                std::cout << "Данные о покрытии записаны в файл coverage.txt" << std::endl;
            }
            break;

        case 3:
            running = false;
            break;

        default:
            std::cout << "❌ Неверный выбор.\n";
            break;
        }
    }

    return 0;
}
// Феодосия телевышка
// 45.0411 35.3556
// 1 45.023686 35.394311
// 2 45.081273 35.356866
// 3 45.051917 35.345444
// 4 45.051047 35.346947
// 5 45.041111 35.355556
// 6 45.039500 35.363333
// 7 45.027728 35.367075
// 8 45.037222 35.380778
// 9 45.057945 35.382683
// 10 45.033023 35.350077