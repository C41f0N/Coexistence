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

    while (window.isOpen())
    {
        const siv::PerlinNoise::seed_type seed = rand();
        const siv::PerlinNoise perlin{seed};

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
            }
        }

        window.clear();

        if (x < 500)
        {
            x++;
        }
        else
        {
            x = 0;
        }

        // Draw the screen
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {

                const double noise = perlin.octave2D_01((i * 0.01), (j * 0.01), 4);
                cout << "Noise: " << noise << endl;
                if (noise > 0.3)
                {
                    screenPixels[i][j]->setFillColor(Color(255, 255, 255, 255));
                }
                else
                {
                    screenPixels[i][j]->setFillColor(Color(255, 255, 255, 0));
                }

                screenPixels[i][j]->setPosition(Vector2f(i, j));

                window.draw(*screenPixels[i][j]);
            }
        }

        window.display();
    }

    return 0;
}