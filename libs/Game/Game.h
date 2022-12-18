#pragma once
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "../../VContainer.h"
#include "../../MError.h"
#include <istream>
namespace marine {
	class Sprite {
	protected:
		std::shared_ptr<sf::Sprite> _this;
	public:
		static Sprite shape(std::string& x) {
			try {
				//retrieve shapes like squares.
			}
			catch (...) {
				throw marine::errors::FileError("That shape does not exist.");
			}
		}
		static Sprite load(std::string x) {
			try {
				sf::Texture texture;
				if (!texture.loadFromFile(x)) {
					throw 1;
				}
				sf::Sprite s(texture);
				return Sprite(&s);
			}
			catch(...) {
				throw marine::errors::RuntimeError("could not load texture from file.");
			}
		}
	public:
		Sprite(sf::Sprite* x): _this(x){}
	};
	class GameObject {

	};
	class GameWindow {
		std::vector<std::shared_ptr<GameObject>> sprites;
		std::shared_ptr<sf::RenderWindow> m_wind;

	public:
		GameWindow(int x, int y, std::string& title) : m_wind(std::make_shared<sf::RenderWindow>(sf::VideoMode(x, y), title)) {}
		void open() {
			while (m_wind->isOpen()) {
				sf::Event event;
				while (m_wind->pollEvent(event))
				{
					// Close window: exit
					if (event.type == sf::Event::Closed)
						m_wind->close();
				}
				m_wind->clear();


				//DRAW LOOP



				m_wind->display();

			}
		}
	};

}