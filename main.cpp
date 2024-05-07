#include <SFML/Graphics.hpp>
#include <iostream>
#include <future>
#include <ctime>
#include <cmath>
#include "PerlinNoise.hpp"

using namespace std;
using namespace sf;

// Setting global variables
const int height = 1080;
const int width = 1920;
int numRabbits = 500;
int rabbitVision = 50;
int frameRate = 60;

const float pi = 3.142;

int hungerlevel = 10;
int thirstlevel = 10;

float rabbitSpeedMin = 0.3;
float rabbitSpeedMax = 0.3;

float deltaTime = 1 / frameRate;
// Objects for background generation and display
Image terrainTextureImage;
Texture terrainTexture;
Sprite backgroundSprite;

class Animal
{
protected:
    CircleShape shape;
    float speed;
    Vector2f direction;
    Vector2f position;
    Vector2f nextPointToRoamTo;

    friend void initializeRabbits(RenderWindow *window);

public:
    Animal() {}

    Animal(
        float speed,
        Vector2f direction,
        Vector2f position)
        : speed(speed),
          direction(direction),
          position(position)
    {
        shape.setPosition(position);
        shape.setRadius(5);
        shape.setFillColor(Color::White);

        nextPointToRoamTo = position;
    }

    virtual void draw(RenderWindow *window) = 0; // virtual function

    int eat()
    {
        hungerlevel -= 2;
        if (hungerlevel < 0)
            hungerlevel = 0;

        return hungerlevel;
    }

    int drink()
    {
        thirstlevel -= 2;
        if (thirstlevel < 0)
            thirstlevel = 0;

        return thirstlevel;
    }

    void roam()
    {
        Vector2f vectorToNextPoint = nextPointToRoamTo - position;
        float distanceToNextPoint = pow(pow(vectorToNextPoint.x, 2) + pow(vectorToNextPoint.y, 2), 0.5);

        if (distanceToNextPoint < 5)
        {
            // Select new point to roam to
            float theta = (((float)(rand() % 1000) / 1000)) * (float)(2 * pi);
            float r = (((float)(rand() % 1000) / 1000)) * rabbitVision;

            float y = position.y + (float)(r * sin(theta));
            float x = position.x + (float)(r * cos(theta));

            nextPointToRoamTo = Vector2f(x, y);
            cout << "Changing direction to (" << x << ", " << y << ")" << endl;
        }
        else
        {
            // Else go to the next point
            direction = Vector2f(vectorToNextPoint.x / distanceToNextPoint, vectorToNextPoint.y / distanceToNextPoint);
        }
    }

    void update()
    {
        roam();

        Vector2f velocity;

        velocity.x = direction.x * speed;
        velocity.y = direction.y * speed;

        position.x = position.x + velocity.x;
        position.y = position.y + velocity.y;

        shape.setPosition(position);
    }

    // bool checkforfood(string food[], int size)
    // {
    //     if (position.x >= 0 && position.x <= 10 && position.y >= 0 && position.y <= 10)
    //     {
    //         for (int i = 0; i < size; i++)
    //         {
    //             if (food[i] == "Plants" || food[i] == "Rabbit" || food[i] == "Wolf")
    //             {
    //                 return true;
    //             }
    //             else
    //             {
    //                 return false;
    //             }
    //         }
    //     }
    // }

    bool checkforWater()
    {

        if (position.x >= 0 && position.x <= 10 && position.y >= 0 && position.y <= 10)
        {
            return true; // Water found
        }
        else
        {
            return false; // No water found
        }
    }
};

class Rabbit : public Animal
{
public:
    Rabbit() {}

    Rabbit(float speed, Vector2f direction, Vector2f position) : Animal(speed, direction, position) {}

    bool checkforPredator()
    {
        if (position.x >= 0 && position.x <= 10 && position.y >= 0 && position.y <= 10)
        {
            return true; // Predator found
        }
        else
        {
            return false; // No predator found
        }
    }

    void draw(RenderWindow *window)
    {
        window->draw(shape);
    }

    bool checkforfood(string food[], int size)
    {
        if (position.x >= 0 && position.x <= 10 && position.y >= 0 && position.y <= 10)
        {
            for (int i = 0; i < size; i++)
            {
                if (food[i] == "Plants")
                {
                    return true;
                }
                else
                    return false;
            }
        }
    }
};

class Wolf : public Animal
{
public:
    Wolf() {}

    Wolf(float speed, Vector2f direction, Vector2f position) : Animal(speed, direction, position) {}

    bool checkforPrey()
    {
        if (position.x >= 0 && position.x <= 10 && position.y >= 0 && position.y <= 10)
        {
            return true; // Prey found
        }
        else
        {
            return false; // No prey found
        }
    }

    void draw(RenderWindow *window)
    {
        window->draw(shape);
    }

    bool checkforfood(string food[], int size)
    {
        if (position.x >= 0 && position.x <= 10 && position.y >= 0 && position.y <= 10)
        {
            for (int i = 0; i < size; i++)
            {
                if (food[i] == "Rabbit")
                {
                    return true;
                }
                else
                    return false;
            }
        }
    }
};

void displayLoadingScreen(RenderWindow *window)
{
    Text text;
    Font font;
    font.loadFromFile("assets/fonts/8-bit-hud.ttf");
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

        while (terrainTextureImage.getPixel(rabbit_x, rabbit_y).b != 0)
        {
            rabbit_x = rand() % window->getSize().x;
            rabbit_y = rand() % window->getSize().y;
        }

        rabbits[i] = new Rabbit((rabbitSpeedMin + ((float)(rand() % 1000) / 1000) * (rabbitSpeedMax - rabbitSpeedMin)), Vector2f(1, 1), Vector2f(rabbit_x, rabbit_y));
        rabbits[i]->shape.setOrigin(rabbits[i]->shape.getGlobalBounds().width / 2,
                                    rabbits[i]->shape.getGlobalBounds().height / 2);
    }
}

int main()
{
    // Initializing random module
    srand(time(NULL));

    RenderWindow window(VideoMode(width, height), "Coexistence");

    window.setKeyRepeatEnabled(false);
    window.setFramerateLimit(frameRate);

    displayLoadingScreen(&window);
    generateTerrain();
    initializeRabbits(&window);

    int i = 0;
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
        window.draw(backgroundSprite);

        for (int i = 0; i < numRabbits; i++)
        {
            rabbits[i]->update();
            rabbits[i]->draw(&window);
        }

        window.display();
    }

    return 0;
}