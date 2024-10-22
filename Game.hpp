#pragma once
#include <SFML/Graphics.hpp>
#include "configuration.hpp"
#include "Star.hpp"

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update(sf::Time);
    void render();
    void handlePlayerInput(sf::Keyboard::Key, bool);
    void updateGeometry(uint32_t, Star const&, sf::VertexArray&);

private:
    sf::RenderWindow mWindow;
    sf::Sprite mPlayer;
    sf::Texture mTexture;
    bool mIsMovingUp = false;
    bool mIsMovingDown = false;
    bool mIsMovingLeft = false;
    bool mIsMovingRight = false;
    float PlayerSpeed = conf::playerSpeed;
    sf::Time TimePerFrame = sf::seconds(conf::dt);
    std::vector<Star> stars;
    sf::Texture starTexture;
    sf::VertexArray va;
    uint32_t first = 0;
};

Game::Game() : 
mWindow(sf::VideoMode(conf::window_size.x, conf::window_size.y), "SFML Application", sf::Style::Fullscreen), 
mPlayer(), mTexture(), stars(), starTexture(), va{ sf::PrimitiveType::Quads, 4 * conf::count }
{
    if (!mTexture.loadFromFile("../Media/Textures/XWing.png"))
    {
        std::cout << "UPS";
    }
    mTexture.setSmooth(true);
    if (!starTexture.loadFromFile("../Media/Textures/star.png"))
    {
        std::cout << "UPS";
    }
    starTexture.setSmooth(true);
    starTexture.generateMipmap();
    mPlayer.setTexture(mTexture);
    mPlayer.setPosition(conf::playerInitialPos.x, conf::playerInitialPos.y);
    mPlayer.setScale(conf::playerScale.x, conf::playerScale.y);

    stars = createStars(conf::count, conf::far);
    auto const texture_size_f = static_cast<sf::Vector2f>(starTexture.getSize());
    for (uint32_t idx{ conf::count }; idx--;)
    {
        uint32_t const i = 4 * idx;
        va[i + 0].texCoords = { 0.0f,0.0f };
        va[i + 1].texCoords = { texture_size_f.x,0.0f };
        va[i + 2].texCoords = { texture_size_f.x,texture_size_f.y };
        va[i + 3].texCoords = { 0.0f,texture_size_f.y };
    }
}

void Game::run()
{
    mWindow.setMouseCursorVisible(false);
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    while (mWindow.isOpen())
    {
        processEvents();
        timeSinceLastUpdate += clock.restart();
        while (timeSinceLastUpdate > TimePerFrame)
        {
            timeSinceLastUpdate -= TimePerFrame;
            processEvents();
            update(TimePerFrame);
        }
        render();
    }
}

void Game::processEvents()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::KeyPressed:
            if (event.key.code == sf::Keyboard::Escape)
            {
                mWindow.close();
            }
            handlePlayerInput(event.key.code, true);
            break;
        case sf::Event::KeyReleased:
            handlePlayerInput(event.key.code, false);
            break;
        case sf::Event::Closed:
            mWindow.close();
            break;
        }
    }
}

void Game::handlePlayerInput(sf::Keyboard::Key key, bool isPressed)
{
    if (key == sf::Keyboard::W)
        mIsMovingUp = isPressed;
    else if (key == sf::Keyboard::S)
        mIsMovingDown = isPressed;
    else if (key == sf::Keyboard::A)
        mIsMovingLeft = isPressed;
    else if (key == sf::Keyboard::D)
        mIsMovingRight = isPressed;
}

void Game::update(sf::Time deltaTime)
{
    sf::Vector2f movement(0.f, 0.f);
    if (mIsMovingUp)
        movement.y -= PlayerSpeed;
    if (mIsMovingDown)
        movement.y += PlayerSpeed;
    if (mIsMovingLeft)
        movement.x -= PlayerSpeed;
    if (mIsMovingRight)
        movement.x += PlayerSpeed;

    mPlayer.move(movement * deltaTime.asSeconds());

    // Star fake travel toward increasing Z
    for (uint32_t i{ conf::count }; i--;)
    {
        Star& s = stars[i];
        s.z -= deltaTime.asSeconds() * conf::starSpeed;
        if (s.z < conf::near)
        {
            // Offset the star by the excess travel it made behind near to keep spacing constant
            s.z = conf::far - (conf::near - s.z);
            // This star is now the first we need to draw because it is  the further away from us
            first = i;
        }
    }
}

void Game::render()
{
    mWindow.clear();

    for (uint32_t i{ 0 }; i < conf::count; ++i)
    {
        uint32_t const idx = (i + first) % conf::count;
        Star const& s = stars[idx];
        updateGeometry(i, s, va);
    }

    sf::RenderStates states;
    states.transform.translate(conf::window_size_f * 0.5f);
    states.texture = &starTexture;
    mWindow.draw(va, states);

    mWindow.draw(mPlayer);

    mWindow.display();
}

void Game::updateGeometry(uint32_t idx, Star const& s, sf::VertexArray& va)
{
    float const scale = 1.0f / s.z;
    float const depth_ratio = (s.z - conf::near) / (conf::far - conf::near);
    float const color_ratio = 1.0f - depth_ratio;
    auto const c = static_cast<uint8_t>(color_ratio * 255.0f);

    sf::Vector2f const p = s.position * scale;
    float const r = conf::radius * scale;
    uint32_t const i = 4 * idx;

    va[i + 0].position = { p.x - r,p.y - r };
    va[i + 1].position = { p.x + r,p.y - r };
    va[i + 2].position = { p.x + r,p.y + r };
    va[i + 3].position = { p.x - r,p.y + r };

    sf::Color const color{ c,c,c };

    va[i + 0].color = color;
    va[i + 1].color = color;
    va[i + 2].color = color;
    va[i + 3].color = color;
}