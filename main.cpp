#include <SFML/Graphics.hpp>
#include <iostream>
#include <future>
#include <ctime>
#include "PerlinNoise.hpp"

using namespace std;
using namespace sf;

// Setting global variables
const int height = 700;
const int width = 700;
int numRabbits = 10;

// Objects for background generation and display
Image terrainTextureImage;
Texture terrainTexture;
Sprite backgroundSprite;

class Rabbit
{
    CircleShape shape;
    float speed = 5;
    float direction[2] = {0, 1};

    friend void initializeRabbits(RenderWindow *window);

public:
    Rabbit(float pos_x, float pos_y, float speed)
    {
        this->speed = speed;
        shape.setPosition(pos_x, pos_y);
        shape.setRadius(5);
        shape.setFillColor(Color::White);

        if (rand() % 10 > 5)
        {
            direction[0] = 1;
        }

        if (rand() % 10 > 5)
        {
            direction[1] = 1;
        }
    }

    void draw(RenderWindow *window)
    {
        window->draw(shape);
    }

    void move()
    {
        float velocity[2] = {
            speed * direction[0],
            speed * direction[1]};

        shape.setPosition(shape.getPosition().x + velocity[0], shape.getPosition().y + velocity[1]);
    }
};

void displayLoadingScreen(RenderWindow *window)
{
    Text text;
    Font font;
    font.loadFromFile("8-bit-hud.ttf");
    text.setFont(font);
    text.setString("Generating\nTerrain...");

    text.setCharacterSize(25);

    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(-bounds.left + bounds.width / 2.f, -bounds.top + bounds.height / 2.f);

    text.setPosition(Vector2f(window->getSize().x / 2, window->getSize().y / 2));
    text.setFillColor(Color::White);

    window->clear();
    window->draw(text);
    window->display();
}

Image *terrainImageP;

Image terrainImage;
// To generate a terrain using perlin noise
void generateTerrain()
{
    terrainTextureImage.create(width, height, sf::Color(0, 0, 0, 0));

    const siv::PerlinNoise::seed_type seed = rand();
    const siv::PerlinNoise perlin{seed};

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            const double noise = perlin.octave2D_01((i * 0.01), (j * 0.01), 1, 0.2);
            if (noise > 0.4)
            {
                terrainTextureImage.setPixel(i, j, Color(0, 100, 0, 255));
            }
            else
            {
                terrainTextureImage.setPixel(i, j, Color(0, 100, 255, 150));
            }
        }
    }

    terrainTexture.loadFromImage(terrainTextureImage);

    backgroundSprite.setTexture(terrainTexture);

    cout << "\nSeed: " << seed << endl;
}

Rabbit **rabbits;

void initializeRabbits(RenderWindow *window)
{

    rabbits = new Rabbit *[numRabbits];

    // Initializing Rabbits
    for (int i = 0; i < numRabbits; i++)
    {
        int rabbit_x = rand() % window->getSize().x;
        int rabbit_y = rand() % window->getSize().y;

        while (terrainImage.getPixel(rabbit_x, rabbit_y).b != 0)
        {
            rabbit_x = rand() % window->getSize().x;
            rabbit_y = rand() % window->getSize().y;
        }

        rabbits[i] = new Rabbit(rabbit_x, rabbit_y, rand() % 50);
        rabbits[i]->shape.setOrigin(rabbits[i]->shape.getGlobalBounds().width / 2, rabbits[i]->shape.getGlobalBounds().height / 2);
    }
}

int main()
{
    // Initializing random module
    srand(time(NULL));

    RenderWindow window(VideoMode(width, height), "Coexistence");

    window.setKeyRepeatEnabled(false);

    displayLoadingScreen(&window);
    generateTerrain();
    fprintf(stderr, "Here\n");
    // initializeRabbits(&window);

    int i = 0;
    while (window.isOpen())
    {
        cout << "printing frame" << ++i << endl;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
            }
        }

        window.clear();
        window.draw(backgroundSprite);

        // for (int i = 0; i < numRabbits; i++)
        // {
        //     rabbits[i]->move();
        //     rabbits[i]->draw(&window);
        // }

        window.display();
    }

    return 0;
}