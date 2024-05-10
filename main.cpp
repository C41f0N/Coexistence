#include <SFML/Graphics.hpp>
#include <iostream>
#include <future>
#include <ctime>
#include <cmath>
#include <vector>
#include "PerlinNoise.hpp"

using namespace std;
using namespace sf;

// Char identifiers for entities on screen
char waterCharIdentifier = 'w';
char landCharIdentifier = 'l';
char rabbitCharIdentifier = 'r';
char foodCharIdentifier = 'f';

// Setting global variables
const int height = 1080;
const int width = 1920;
int numRabbits = 500;
float rabbitSize = 3;
int rabbitVision = 50;
int frameRate = 60;

int rabbitsCapacity = 1;

float foodSize = 2;
float foodDensity = 0.0001;

int numFoods = floor((width * height) * foodDensity);

int landColorRGBA[4] = {1, 99, 0, 255};
int waterColorRGBA[4] = {6, 54, 137, 255};

const float pi = 3.142;

int hungerlevel = 10;
int thirstlevel = 10;

float rabbitSpeedMin = 0.1;
float rabbitSpeedMax = 0.2;

float deltaTime = 1 / frameRate;

class Rabbit;
class Food;

vector<Rabbit *> rabbits;

// Objects for terrain generation and display
Image terrainTextureImage;
Texture terrainTexture;
Sprite backgroundSprite;

// Array that holds the position data for entities
vector<char> positionBlueprint[width][height];

bool terrainGenerated = false;

// Function headers
bool isLand(int x, int y);
bool isWithinBounds(int x, int y);
void addToPositionBlueprint(char charIdentifier, int x, int y);
bool checkPositionInBlueprint(char charIdentifier, int x, int y);
void removePositionFromBlueprint(char charIdentifier, int x, int y);

class Animal
{
protected:
    CircleShape shape;          // SFML Shape object for the animal
    float speed;                // Speed of the animal
    Vector2f direction;         // Direction the animal is headed
    Vector2f position;          // Current position of the animal
    Vector2f nextPointToRoamTo; // Direction the animal is headed when roaming randomly

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
        // Setting the shape origin to center
        shape.setOrigin(shape.getGlobalBounds().width / 2, shape.getGlobalBounds().height / 2);

        shape.setPosition(floor(position.x), floor(position.y));

        nextPointToRoamTo = position;
    }

    virtual void draw(RenderWindow *window) = 0; // virtual function

    Vector2f getPosition()
    {
        return position;
    }

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
        if (terrainGenerated)
        {
            Vector2f vectorToNextPoint = nextPointToRoamTo - position;
            float distanceToNextPoint = pow(pow(vectorToNextPoint.x, 2) + pow(vectorToNextPoint.y, 2), 0.5);

            if (distanceToNextPoint < 5)
            {
                // Select new point to roam to

                int x, y;

                do
                {
                    float theta = (((float)(rand() % 1000) / 1000)) * (float)(2 * pi);
                    float r = (((float)(rand() % 1000) / 1000)) * rabbitVision;

                    x = (int)round(position.x + (float)(r * cos(theta)));
                    y = (int)round(position.y + (float)(r * sin(theta)));

                } while (!isLand(x, y));

                nextPointToRoamTo = Vector2f(x, y);
            }
            else
            {
                // Else go to the next point
                direction = Vector2f(vectorToNextPoint.x / distanceToNextPoint, vectorToNextPoint.y / distanceToNextPoint);
            }
        }
    }

    virtual void move()
    {
    }

    void update()
    {
        roam();
        move();
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

    // Gonna search starting from the center, and searching in a circle, like the circle grows wider, this way we van ge the closest source of water / land
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
    Rabbit(float speed, Vector2f direction, Vector2f position) : Animal(speed, direction, position)
    {
        shape.setRadius(rabbitSize);
        shape.setFillColor(Color::White);

        // Put shape position in position blueprint
        addToPositionBlueprint('r', floor(position.x), floor(position.y));
    }

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

    void move() override
    {

        Vector2f velocity;

        velocity.x = direction.x * speed;
        velocity.y = direction.y * speed;

        // Remove old position blueprint
        removePositionFromBlueprint('r', floor(position.x), floor(position.y));

        position.x = position.x + velocity.x;
        position.y = position.y + velocity.y;

        addToPositionBlueprint('r', floor(position.x), floor(position.y));

        shape.setPosition(position);
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

class Food
{
    CircleShape shape;
    int capacity;
    Vector2f position;

public:
    Food(Vector2f position) : capacity(5)
    {
        shape.setRadius(foodSize);
        shape.setFillColor(Color::Yellow);
        shape.setPosition(position);

        // registering food to blueprint
        addToPositionBlueprint('f', floor(position.x), floor(position.y));

        // Centering the shape's origin
        shape.setOrigin(shape.getGlobalBounds().width / 2, shape.getGlobalBounds().height / 2);
    }

    void eat()
    {
        capacity--;

        if (capacity <= 0)
        {
            removePositionFromBlueprint('f', position.x, position.y);
        }
    }

    void draw(RenderWindow *window)
    {
        if (capacity > 0)
        {
            window->draw(shape);
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

// ------------ TERRAIN FUNCTIONS ----------------

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
                addToPositionBlueprint('l', i, j);
                terrainTextureImage.setPixel(i, j, Color(landColorRGBA[0], landColorRGBA[1], landColorRGBA[2], landColorRGBA[3]));
            }
            else
            {
                addToPositionBlueprint('w', i, j);
                terrainTextureImage.setPixel(i, j, Color(waterColorRGBA[0], waterColorRGBA[1], waterColorRGBA[2], waterColorRGBA[3]));
            }
        }
    }

    terrainTexture.loadFromImage(terrainTextureImage);

    backgroundSprite.setTexture(terrainTexture);

    terrainGenerated = true;

    cout << "\nSeed: " << seed << endl;
}

// ------------ POSITION BLUEPRINT FUNCTIONS ----------------

void addToPositionBlueprint(char charIdentifier, int x, int y)
{
    // Adding the char to the blueprint
    positionBlueprint[x][y].push_back(charIdentifier);
}

bool checkPositionInBlueprint(char charIdentifier, int x, int y)
{
    for (int i = 0; i < positionBlueprint[x][y].size(); i++)
    {
        if (positionBlueprint[x][y][i] == charIdentifier)
        {
            return true;
        }
    }

    return false;
}

void removePositionFromBlueprint(char charIdentifier, int x, int y)
{
    int existsAt = -1;

    // Checking if charIdentifier exists on the coordinates
    for (int i = 0; i < positionBlueprint[x][y].size(); i++)
    {
        if (positionBlueprint[x][y][i] == charIdentifier)
        {
            existsAt = i;
            break;
        }
    }

    // Removing position from blueprint if it exists
    if (existsAt != -1)
    {
        vector<char>::iterator it = positionBlueprint[x][y].begin();
        advance(it, existsAt);

        positionBlueprint[x][y].erase(it);
    }
}

// ------------ UTILITY FUNCTIONS ----------------

bool isLand(int x, int y)
{
    if (isWithinBounds(x, y))
    {
        Color thisPixel = terrainTextureImage.getPixel(x, y);

        return thisPixel.r == landColorRGBA[0] && thisPixel.g == landColorRGBA[1] && thisPixel.b == landColorRGBA[2] && thisPixel.a == landColorRGBA[3];
    }
    else
    {
        return false;
    }
}

bool isWithinBounds(int x, int y)
{

    return (x < width && x > 0 && y < height && y > 0);
}

// ------------ FUNCTIONS FOR RABBITS ------------------

void addRabbit(Vector2f position)
{
    int rabbit_x = floor(position.x);
    int rabbit_y = floor(position.y);

    Rabbit *rabbit = new Rabbit((rabbitSpeedMin + ((float)(rand() % 1000) / 1000) * (rabbitSpeedMax - rabbitSpeedMin)),
                                Vector2f(1, 1),
                                Vector2f(rabbit_x, rabbit_y));

    rabbits.push_back(rabbit);
}

void removeRabbit(Vector2f position)
{

    int targetAt = -1;

    for (int i = 0; i < rabbits.size(); i++)
    {
        if (rabbits[i]->getPosition() == position)
        {
            targetAt = i;
            break;
        }
    }

    if (targetAt >= 0 && targetAt <= rabbits.size())
    {
        vector<Rabbit *>::iterator it = rabbits.begin();

        advance(it, targetAt);

        rabbits.erase(it);
    }
}

void initializeRabbits()
{
    for (int i = 0; i < numRabbits; i++)
    {

        int rabbit_x;
        int rabbit_y;

        do
        {
            rabbit_x = rand() % width;
            rabbit_y = rand() % height;
        } while (!isLand(rabbit_x, rabbit_y));

        addRabbit(Vector2f((float)rabbit_x, (float)rabbit_y));
    }
}

void updateAllRabbits()
{
    for (int i = 0; i < rabbits.size(); i++)
    {
        rabbits[i]->update();
    }
}

void drawAllRabbits(RenderWindow *window)
{

    for (int i = 0; i < rabbits.size(); i++)
    {
        rabbits[i]->draw(window);
    }
}

// ------------- MASTER FUNCTIONS -----------------------

void masterUpdate()
{
    updateAllRabbits();
}

void masterDraw(RenderWindow *window)
{
    // Draw the terrain
    window->draw(backgroundSprite);

    // Draw the rabbits
    drawAllRabbits(window);
}

void masterInitialize()
{

    generateTerrain();
    initializeRabbits();
}

int main()
{
    // Initializing random module
    srand(time(NULL));

    RenderWindow window(VideoMode(width, height), "Coexistence");

    masterInitialize();

    window.setKeyRepeatEnabled(false);
    window.setFramerateLimit(frameRate);

    displayLoadingScreen(&window);

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

        masterUpdate();

        window.clear();

        masterDraw(&window);

        window.display();
    }

    return 0;
}