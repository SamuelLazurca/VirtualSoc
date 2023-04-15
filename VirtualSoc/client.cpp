#include "ClientAction.h"

/*#include <SFML/Graphics.hpp>

int main(){
    sf::RenderWindow window(sf::VideoMode(800,600),"Window",
                            sf::Style::Titlebar | sf::Style::Close);
    sf::Font arial;
    arial.loadFromFile("../arial.ttf");
    sf::Text t;
    t.setFillColor(sf::Color::White);
    t.setFont(arial);
    std::string s = "This is text that you type: \n";
    t.setString(s);

    while(window.isOpen()){
        sf::Event event;

        while(window.pollEvent(event)){
            if(event.type == sf::Event::Closed){
                window.close();
            }
            if (event.type == sf::Event::TextEntered){
                if (event.text.unicode < 128){
                    s += static_cast<char>(event.text.unicode);
                }
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Enter)
                {
                    s += "\n";
                    std::cout << "the enter key was pressed" << std::endl;
                }
            }

        }

        if(s.length()%40 == 0) s+= "\n";
        t.setString(s);
        window.clear(sf::Color::Black);
        window.draw(t);
        window.display();
    }
}*/

int main (int argc, char *argv[])
{
    if (argc != 3)
    {
        ClientAction c("127.0.0.1", "2728");
        c.Run();
    }
    else
    {
        ClientAction c(argv[1], argv[2]);
        c.Run();
    }

}