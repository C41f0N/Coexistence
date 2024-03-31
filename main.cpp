#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include "PerlinNoise.hpp"

using namespace std;
using namespace sf;

int main()
{

    srand(time(NULL));

    const int height = 700;
    const int width = 700;

    RenderWindow window(VideoMode(width, height), "Coexistence");

    int x = 0;

    // Pixel Array
    RectangleShape *screenPixels[height][width];

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            screenPixels[i][j] = new RectangleShape(Vector2f(1, 1));
        }
    }

    const siv::PerlinNoise::seed_type seed = rand();
    const siv::PerlinNoise perlin{seed};

    // Generating terrain
    cout << "Generating Terrain" << endl;
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            const double noise = perlin.octave2D_01((i * 0.01), (j * 0.01), 1, 0.2);
            // cout << "[" << i << ", " << j << "] : " << noise << endl;
            if (noise > 0.4)
            {
                screenPixels[i][j]->setFillColor(Color(0, 200, 0, 200));
            }
            else
            {
                screenPixels[i][j]->setFillColor(Color(0, 100, 255, 150));
            }
            cout << round((i * 100) / width) << "%\n";
        }
    }

    cout << "\nSeed: " << seed << endl;

    while (window.isOpen())
    {

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
            }
        }

        window.clear();

        // Draw the screen
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                screenPixels[i][j]->setPosition(Vector2f(i, j));
                window.draw(*screenPixels[i][j]);
            }
        }

        window.display();
    }

    return 0;
}