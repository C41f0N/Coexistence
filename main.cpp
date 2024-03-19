#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;
using namespace sf;

int main()
{

    const int height = 500;
    const int width = 500;

    int size = 100;

    RenderWindow window(VideoMode(width, height), "Coexistence");
    CircleShape shape(size);
    shape.setPosition(Vector2f(((int)(width / 2)) - ((int)(size)),
                               ((int)(height / 2)) - ((int)(size))));

    shape.setFillColor(Color::Blue);

    int i = 0;

    while (window.isOpen())
    {

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }
        i++;
        cout << i << endl;
        window.clear();
        // shape.setScale(Vector2f(i + 100, i));
        shape.setPosition(shape.getPosition().x + (int)(i / 200),
                          shape.getPosition().y + (int)(i / 200));

        window.draw(shape);
        window.display();
    }

    return 0;
}