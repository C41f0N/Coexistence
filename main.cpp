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
char plantCharIdentifier = 'p';
char wolfCharIdentifier = 'W';

// Setting global variables
const int height = 1080 / 2;
const int width = 1920 / 2;

// -------- RABBIT VARIABLES ----------
int intialNumRabbits = 30;
float rabbitRadius = 3;
int rabbitVision = 30;

float rabbitMaxHunger = 50;
float rabbitMaxThirst = 80;
float rabbitMaxReproductiveUrge = 100;

float rabbitHungerDelta = 0.05;
float rabbitThirstDelta = 0.05;
float rabbitReproductiveUrgeDelta = 0.05;

float rabbitSpeedMin = 1;
float rabbitSpeedMax = 1;

// -------- WOLF VARIABLES ----------
int initialNumWolves = 30;
float wolfRadius = 4;
int wolfVision = 40;

float wolfMaxHunger = 100;
float wolfMaxThirst = 80;
float wolfMaxReproductiveUrge = 100;

float wolfHungerDelta = 0.05;
float wolfThirstDelta = 0.05;
float wolfReproductiveUrgeDelta = 0.05;

float wolfSpeedMin = 1;
float wolfSpeedMax = 1;

// -------- PLANT VARIABLES ----------
float plantDensity = 4;
float plantSize = 5;
int numPlants = floor((width * height) * plantDensity);

// -------- COLOR VARIABLES ----------
int landColorRGBA[4] = {1, 99, 0, 255};
int waterColorRGBA[4] = {6, 54, 137, 255};

// -------- OTHER VARIABLES ----------
int frameRate = 30;

const float pi = 3.142;

float deltaTime = 1 / frameRate;

class Rabbit;
class Plant;
class Wolf;

vector<Rabbit *> rabbits;
vector<Plant *> plants;
vector<Wolf *> wolves;

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

void removePlant(Vector2f position);
void addRabbit(Vector2f position);
void removeRabbit(Vector2f position);
void addWolf(Vector2f position);
void removeWolf(Vector2f position);

// ----------------- CLASSES ------------------

class Animal
{
protected:
    CircleShape shape;  // SFML Shape object for the animal
    float speed;        // Speed of the animal
    Vector2f direction; // Direction the animal is headed
    Vector2f position;  // Current position of the animal
    Vector2f headedTo;  // Direction the animal is headed when roaming randomly
    Vector2f closestFoodSource;
    Vector2f closestWaterSource;
    Vector2f closestMate;
    float maxHunger;
    float maxThirst;
    float maxReproductiveUrge;
    float hungerLevel;
    float thirstLevel;
    float reproductiveUrge;

    friend void initializeRabbits(RenderWindow *window);

public:
    Animal(
        float speed,
        Vector2f direction,
        Vector2f position,
        float maxHunger,
        float maxThirst,
        float maxReproductiveUrge)
        : speed(speed),
          direction(direction),
          position(position),
          maxHunger(maxHunger),
          maxThirst(maxThirst),
          maxReproductiveUrge(maxReproductiveUrge),
          closestFoodSource(Vector2f(-1, -1)),
          closestWaterSource(Vector2f(-1, -1)),
          closestMate(Vector2f(-1, -1))
    {
        // Setting the shape origin to center
        shape.setOrigin(shape.getGlobalBounds().width / 2, shape.getGlobalBounds().height / 2);

        shape.setPosition(floor(position.x), floor(position.y));

        headedTo = position;
    }

    virtual void draw(RenderWindow *window) = 0; // virtual function

    Vector2f getPosition()
    {
        return position;
    }

    void roam()
    {
        if (terrainGenerated)
        {
            Vector2f vectorToNextPoint = headedTo - position;
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

                headedTo = Vector2f(x, y);
            }
        }
    }

    virtual void move()
    {
        // Basic move funcion that just sets the animal's direction to its next goal
        Vector2f vectorToNextPoint = headedTo - position;
        float distanceToNextPoint = pow(pow(vectorToNextPoint.x, 2) + pow(vectorToNextPoint.y, 2), 0.5);

        direction = Vector2f(vectorToNextPoint.x / distanceToNextPoint, vectorToNextPoint.y / distanceToNextPoint);
    }

    void update()
    {
        roam();
        move();
    }

    virtual bool checkforplant()
    {
        return false;
    }

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
protected:
    Vector2f threatsAverageLocation;

public:
    Rabbit(
        float speed,
        Vector2f direction,
        Vector2f position,
        float maxHunger,
        float maxThirst,
        float maxReproductiveUrge)
        : Animal(
              speed,
              direction,
              position,
              maxHunger,
              maxThirst,
              maxReproductiveUrge),
          threatsAverageLocation(Vector2f(-1, -1))
    {
        shape.setRadius(rabbitRadius);
        shape.setFillColor(Color::White);
        shape.setOrigin(shape.getGlobalBounds().width / 2, shape.getGlobalBounds().height / 2);

        // Setting random values for thirst hunger and mating urge
        hungerLevel = (float)(rand() % (int)(rabbitMaxHunger));
        thirstLevel = (float)(rand() % (int)(rabbitMaxThirst));
        reproductiveUrge = (float)(rand() % (int)(rabbitMaxReproductiveUrge));
    }

    void move() override
    {

        Animal::move();

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

    bool atPlant()
    {
        bool found = false;
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                found = checkPositionInBlueprint(plantCharIdentifier, position.x + i - 2, position.y + j - 2);
                if (found)
                    return found;
            }
        }
        return found;
    }

    bool atWater()
    {
        bool found = false;
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                found = checkPositionInBlueprint(waterCharIdentifier, position.x + i - 2, position.y + j - 2);
                if (found)
                    return found;
            }
        }
        return found;
    }

    bool atMate()
    {
        bool found = false;
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                if (i != 0 && j != 0)
                {
                    found = checkPositionInBlueprint(rabbitCharIdentifier, position.x + i - 2, position.y + j - 2);
                    if (found)
                        return found;
                }
            }
        }

        return found;
    }

    void scanSurroundings()
    {

        closestFoodSource = Vector2f(-1, -1);
        closestWaterSource = Vector2f(-1, -1);
        closestMate = Vector2f(-1, -1);
        threatsAverageLocation = Vector2f(-1, -1);

        // Scan suroundings and look for plant
        for (int r = 0; r < rabbitVision + 1; r++)
        {
            // First checking the pixel the rabbit is currently on
            if (r == 0)
            {
                int search_x = round(position.x);
                int search_y = round(position.y);

                // If plant found and plant not already found
                if (checkPositionInBlueprint(plantCharIdentifier, search_x, search_y) && closestFoodSource == Vector2f(-1, -1))
                {
                    closestFoodSource = Vector2f(search_x, search_y);
                }

                // If water found
                if (checkPositionInBlueprint(waterCharIdentifier, search_x, search_y) && closestWaterSource == Vector2f(-1, -1))
                {
                    closestWaterSource = Vector2f(search_x, search_y);
                }

                // If wolf found
                if (checkPositionInBlueprint(wolfCharIdentifier, search_x, search_y))
                {
                    if (threatsAverageLocation == Vector2f(-1, -1))
                    {
                        threatsAverageLocation = Vector2f(search_x, search_y);
                    }
                    else
                    {
                        threatsAverageLocation = Vector2f((float)round((threatsAverageLocation.x + search_x) / 2),
                                                          (float)round((threatsAverageLocation.y + search_y) / 2));
                    }
                }
            }
            // Checking all other pixels in the rabbit's field of vision other than its own position
            else
            {
                float dtheta = (float)(1 / (float)(2 * r));
                for (float theta = 0; theta < (2 * pi); theta += dtheta)
                {
                    int search_x = round(position.x + r * cos(theta));
                    int search_y = round(position.y + r * sin(theta));

                    // If plant found and plant not already found
                    if (checkPositionInBlueprint(plantCharIdentifier, search_x, search_y) && closestFoodSource == Vector2f(-1, -1))
                    {
                        closestFoodSource = Vector2f(search_x, search_y);
                    }

                    // If water found
                    if (checkPositionInBlueprint(waterCharIdentifier, search_x, search_y) && closestWaterSource == Vector2f(-1, -1))
                    {
                        closestWaterSource = Vector2f(search_x, search_y);
                    }

                    // If mate found
                    if (checkPositionInBlueprint(rabbitCharIdentifier, search_x, search_y) && closestMate == Vector2f(-1, -1))
                    {
                        if (floor(position.x) != search_x || floor(position.y) != search_y)
                        {

                            closestMate = Vector2f(search_x, search_y);
                        }
                    }

                    // If wolf found
                    if (checkPositionInBlueprint(wolfCharIdentifier, search_x, search_y))
                    {
                        if (threatsAverageLocation == Vector2f(-1, -1))
                        {
                            threatsAverageLocation = Vector2f(search_x, search_y);
                        }
                        else
                        {
                            threatsAverageLocation = Vector2f((float)round((threatsAverageLocation.x + search_x) / 2),
                                                              (float)round((threatsAverageLocation.y + search_y) / 2));
                        }
                    }
                }
            }
        }
    }

    bool checkforplant()
    {
        // Scan suroundings and look for plant
        for (int r = 0; r < rabbitVision + 1; r++)
        {

            // First checking the pixel the rabbit is currently on
            if (r == 0)
            {
                int search_x = round(position.x);
                int search_y = round(position.y);

                // If plant found
                if (checkPositionInBlueprint('p', search_x, search_y))
                {
                    headedTo = Vector2f((float)search_x, (float)search_y);
                    return true;
                }
            }
            // Checking all other pixels in the rabbit's field of vision
            else
            {
                float dtheta = (float)(1 / (float)(2 * r));
                for (float theta = 0; theta < (2 * pi); theta += dtheta)
                {
                    int search_x = round(position.x + r * cos(theta));
                    int search_y = round(position.y + r * sin(theta));

                    // If plant found
                    if (checkPositionInBlueprint('p', search_x, search_y))
                    {
                        headedTo = Vector2f((float)search_x, (float)search_y);
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void update()
    {

        hungerLevel += rabbitHungerDelta;
        thirstLevel += rabbitThirstDelta;
        reproductiveUrge += rabbitReproductiveUrgeDelta;

        scanSurroundings();

        // If hunger level is down then check for plant before roaming
        if (hungerLevel > (float)(maxHunger / 2) && closestFoodSource != Vector2f(-1, -1))
        {
            headedTo = closestFoodSource;

            if (atPlant())
            {
                hungerLevel = 0;
            }
        }
        else if (thirstLevel > (float)(maxThirst / 2) && closestWaterSource != Vector2f(-1, -1))
        {
            headedTo = closestWaterSource;

            if (atWater())
            {
                thirstLevel = 0;
            }
        }
        else if (reproductiveUrge > (float)(maxReproductiveUrge / 2) && closestMate != Vector2f(-1, -1))
        {
            headedTo = closestMate;

            if (atMate())
            {
                reproductiveUrge = 0;
                // CREATE BABY
                addRabbit(position);
            }
        }
        else
        {
            roam();
        }

        // Kill if too much hunger
        if (hungerLevel > maxHunger || thirstLevel > maxThirst)
        {
            removeRabbit(position);
        }

        move();
    }
};

class Wolf : public Animal
{
protected:
    Vector2f threatsAverageLocation;

public:
    Wolf(
        float speed,
        Vector2f direction,
        Vector2f position,
        float maxHunger,
        float maxThirst,
        float maxReproductiveUrge)
        : Animal(
              speed,
              direction,
              position,
              maxHunger,
              maxThirst,
              maxReproductiveUrge),
          threatsAverageLocation(Vector2f(-1, -1))
    {
        shape.setRadius(wolfRadius);
        shape.setFillColor(Color::Red);
        shape.setOrigin(shape.getGlobalBounds().width / 2, shape.getGlobalBounds().height / 2);

        // Setting random values for thirst hunger and mating urge
        hungerLevel = (float)(rand() % (int)(wolfMaxHunger));
        thirstLevel = (float)(rand() % (int)(wolfMaxThirst));
        reproductiveUrge = (float)(rand() % (int)(wolfMaxReproductiveUrge));
    }

    void move() override
    {

        Animal::move();

        Vector2f velocity;

        velocity.x = direction.x * speed;
        velocity.y = direction.y * speed;

        // Remove old position blueprint
        removePositionFromBlueprint(wolfCharIdentifier, floor(position.x), floor(position.y));

        position.x = position.x + velocity.x;
        position.y = position.y + velocity.y;

        addToPositionBlueprint(wolfCharIdentifier, floor(position.x), floor(position.y));

        shape.setPosition(position);
    }

    void draw(RenderWindow *window)
    {
        window->draw(shape);
    }

    bool atRabbit()
    {

        return checkPositionInBlueprint(rabbitCharIdentifier, position.x, position.y);
    }

    bool atWater()
    {
        bool found = false;
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                found = checkPositionInBlueprint(waterCharIdentifier, position.x + i - 2, position.y + j - 2);
                if (found)
                    return found;
            }
        }
        return found;
    }

    bool atMate()
    {
        bool found = false;
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                if (i != 0 && j != 0)
                {
                    found = checkPositionInBlueprint(wolfCharIdentifier, position.x + i - 2, position.y + j - 2);
                    if (found)
                        return found;
                }
            }
        }

        return found;
    }

    void scanSurroundings()
    {

        closestFoodSource = Vector2f(-1, -1);
        closestWaterSource = Vector2f(-1, -1);
        closestMate = Vector2f(-1, -1);

        // Scan suroundings and look for plant
        for (int r = 0; r < wolfVision + 1; r++)
        {
            // First checking the pixel the wolf is currently on
            if (r == 0)
            {
                int search_x = round(position.x);
                int search_y = round(position.y);

                // If rabbit found and plant not already found
                if (checkPositionInBlueprint(rabbitCharIdentifier, search_x, search_y) && closestFoodSource == Vector2f(-1, -1))
                {
                    closestFoodSource = Vector2f(search_x, search_y);
                }

                // If water found
                if (checkPositionInBlueprint(waterCharIdentifier, search_x, search_y) && closestWaterSource == Vector2f(-1, -1))
                {
                    closestWaterSource = Vector2f(search_x, search_y);
                }
            }
            // Checking all other pixels in the wolf's field of vision other than its own position
            else
            {
                float dtheta = (float)(1 / (float)(2 * r));
                for (float theta = 0; theta < (2 * pi); theta += dtheta)
                {
                    int search_x = round(position.x + r * cos(theta));
                    int search_y = round(position.y + r * sin(theta));

                    // If rabbit found and plant not already found
                    if (checkPositionInBlueprint(rabbitCharIdentifier, search_x, search_y) && closestFoodSource == Vector2f(-1, -1))
                    {
                        closestFoodSource = Vector2f(search_x, search_y);
                    }

                    // If water found
                    if (checkPositionInBlueprint(waterCharIdentifier, search_x, search_y) && closestWaterSource == Vector2f(-1, -1))
                    {
                        closestWaterSource = Vector2f(search_x, search_y);
                    }

                    // If mate found
                    if (checkPositionInBlueprint(wolfCharIdentifier, search_x, search_y) && closestMate == Vector2f(-1, -1))
                    {
                        if (floor(position.x) != search_x || floor(position.y) != search_y)
                        {

                            closestMate = Vector2f(search_x, search_y);
                        }
                    }
                }
            }
        }
    }

    void update()
    {

        hungerLevel += wolfHungerDelta;
        thirstLevel += wolfThirstDelta;
        reproductiveUrge += wolfReproductiveUrgeDelta;

        scanSurroundings();

        // If hunger level is down then check for plant before roaming
        if (hungerLevel > (float)(maxHunger / 2) && closestFoodSource != Vector2f(-1, -1))
        {
            headedTo = closestFoodSource;

            if (atRabbit())
            {
                hungerLevel = 0;

                removeRabbit(Vector2f(round(position.x), round(position.y)));
            }
        }
        else if (thirstLevel > (float)(maxThirst / 2) && closestWaterSource != Vector2f(-1, -1))
        {
            headedTo = closestWaterSource;

            if (atWater())
            {
                thirstLevel = 0;
            }
        }
        else if (reproductiveUrge > (float)(maxReproductiveUrge / 2) && closestMate != Vector2f(-1, -1))
        {
            headedTo = closestMate;

            if (atMate())
            {
                reproductiveUrge = 0;
                // CREATE BABY
                addWolf(position);
            }
        }
        else
        {
            roam();
        }

        // Kill if too much hunger
        if (hungerLevel > maxHunger || thirstLevel > maxThirst)
        {
            removeWolf(position);
        }

        move();
    }
};

class Plant
{
    CircleShape shape;
    Vector2f position;

public:
    Plant(Vector2f position) : position(position)
    {
        shape.setRadius(plantSize);
        // Centering the shape's origin
        shape.setOrigin(shape.getGlobalBounds().width / 2, shape.getGlobalBounds().height / 2);
        shape.setPosition(floor(position.x), floor(position.y));

        shape.setFillColor(Color::Green);
    }

    Vector2f getPosition()
    {
        return this->position;
    }

    void draw(RenderWindow *window)
    {
        window->draw(shape);
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
    if (isWithinBounds(x, y))
    {
        positionBlueprint[x][y].push_back(charIdentifier);
    }
}

bool checkPositionInBlueprint(char charIdentifier, int x, int y)
{
    if (isWithinBounds(x, y))
    {
        for (int i = 0; i < positionBlueprint[x][y].size(); i++)
        {
            if (positionBlueprint[x][y][i] == charIdentifier)
            {
                return true;
            }
        }
    }

    return false;
}

void removePositionFromBlueprint(char charIdentifier, int x, int y)
{
    if (isWithinBounds(x, y))
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
    // int rabbit_x = floor(width / 2);
    // int rabbit_y = floor(height / 2);

    Rabbit *rabbit = new Rabbit((rabbitSpeedMin + ((float)(rand() % 1000) / 1000) * (rabbitSpeedMax - rabbitSpeedMin)),
                                Vector2f(1, 1),
                                Vector2f(rabbit_x, rabbit_y),
                                rabbitMaxHunger,
                                rabbitMaxThirst,
                                rabbitMaxReproductiveUrge);

    addToPositionBlueprint('r', floor(position.x), floor(position.y));
    rabbits.push_back(rabbit);
}

void removeRabbit(Vector2f position)
{

    int targetAt = -1;

    for (int i = 0; i < rabbits.size(); i++)
    {
        if (Vector2f(floor(rabbits[i]->getPosition().x), floor(rabbits[i]->getPosition().y)) == Vector2f(floor(position.x), floor(position.y)))
        {
            targetAt = i;
            break;
        }
    }

    if (targetAt >= 0 && targetAt <= rabbits.size())
    {
        vector<Rabbit *>::iterator it = rabbits.begin();

        removePositionFromBlueprint(rabbitCharIdentifier, rabbits[targetAt]->getPosition().x, rabbits[targetAt]->getPosition().y);

        advance(it, targetAt);
        rabbits.erase(it);
    }
}

void initializeRabbits()
{
    for (int i = 0; i < intialNumRabbits; i++)
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

// ------------ FUNCTIONS FOR WOLVES ------------------

void addWolf(Vector2f position)
{
    int wolf_x = floor(position.x);
    int wolf_y = floor(position.y);
    // int wolf_x = floor(width / 2);
    // int wolf_y = floor(height / 2);

    Wolf *wolf = new Wolf((wolfSpeedMin + ((float)(rand() % 1000) / 1000) * (wolfSpeedMax - wolfSpeedMin)),
                          Vector2f(1, 1),
                          Vector2f(wolf_x, wolf_y),
                          wolfMaxHunger,
                          wolfMaxThirst,
                          wolfMaxReproductiveUrge);

    addToPositionBlueprint(wolfCharIdentifier, floor(position.x), floor(position.y));
    wolves.push_back(wolf);
}

void removeWolf(Vector2f position)
{

    int targetAt = -1;

    for (int i = 0; i < wolves.size(); i++)
    {
        if (wolves[i]->getPosition() == position)
        {
            targetAt = i;
            break;
        }
    }

    if (targetAt >= 0 && targetAt <= wolves.size())
    {
        vector<Wolf *>::iterator it = wolves.begin();

        removePositionFromBlueprint(wolfCharIdentifier, wolves[targetAt]->getPosition().x, wolves[targetAt]->getPosition().y);

        advance(it, targetAt);
        wolves.erase(it);
    }
}

void initializeWolves()
{
    for (int i = 0; i < initialNumWolves; i++)
    {

        int wolf_x;
        int wolf_y;

        do
        {
            wolf_x = rand() % width;
            wolf_y = rand() % height;
        } while (!isLand(wolf_x, wolf_y));

        addWolf(Vector2f((float)wolf_x, (float)wolf_y));
    }
}

void updateAllWolves()
{
    for (int i = 0; i < wolves.size(); i++)
    {
        wolves[i]->update();
    }
}

void drawAllWolves(RenderWindow *window)
{

    for (int i = 0; i < wolves.size(); i++)
    {
        wolves[i]->draw(window);
    }
}

// ------------- PLANT FUNCTIONS -----------------------
void addPlant(Vector2f position)
{
    Plant *plant = new Plant(position);

    addToPositionBlueprint('p', floor(position.x), floor(position.y));
    plants.push_back(plant);
}

void removePlant(Vector2f position)
{
    int targetAt = -1;

    for (int i = 0; i < plants.size(); i++)
    {
        if (plants[i]->getPosition() == position)
        {
            targetAt = i;
            break;
        }
    }

    if (targetAt >= 0 && targetAt <= plants.size())
    {
        vector<Plant *>::iterator it = plants.begin();

        advance(it, targetAt);

        plants.erase(it);

        removePositionFromBlueprint('p', position.x, position.y);
    }
}

void initializePlant()
{
    int numPlant = (int)(plantDensity * (width * height) / 10000);

    for (int i = 0; i < (int)(numPlant); i++)
    {
        int plant_x;
        int plant_y;

        do
        {
            plant_x = rand() % width;
            plant_y = rand() % height;
        } while (!isLand(plant_x, plant_y));

        addPlant(Vector2f((float)plant_x, (float)plant_y));
    }
}

void drawAllPlants(RenderWindow *window)
{
    for (int i = 0; i < plants.size(); i++)
    {
        plants[i]->draw(window);
    }
}

// ------------- MASTER FUNCTIONS -----------------------

void masterUpdate()
{
    updateAllRabbits();
    updateAllWolves();

    fprintf(stderr, "Rabbits Alive: %ld Wolves Alive: %ld\n", rabbits.size(), wolves.size());
}

void masterDraw(RenderWindow *window)
{
    // Draw the terrain
    window->draw(backgroundSprite);

    // Draw the rabbits
    drawAllRabbits(window);

    // Dray the wolves
    drawAllWolves(window);

    // Draw the plants
    drawAllPlants(window);
}

void masterInitialize()
{

    generateTerrain();
    initializeRabbits();
    initializeWolves();
    initializePlant();
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