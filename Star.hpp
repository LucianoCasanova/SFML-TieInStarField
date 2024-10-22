#pragma once
#include <SFML/System.hpp>
#include <random>

struct Star
{
	sf::Vector2f position;
	float z = 1.0f;
};

std::vector<Star> createStars(uint32_t count, float scale)
{
    std::vector<Star> stars;
    stars.reserve(count);

    //Random numbers generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    // Define a star free zone

    sf::Vector2f const window_world_size = conf::window_size_f * conf::near;
    sf::FloatRect const star_free_zone = { -window_world_size * 0.5f, window_world_size };

    // Create randomly distributed stars on the screen
    for (uint32_t i{ count }; i--;)
    {
        float const x = (dis(gen) - 0.5f) * conf::window_size_f.x * scale;
        float const y = (dis(gen) - 0.5f) * conf::window_size_f.y * scale;
        float const z = dis(gen) * (conf::far - conf::near) + conf::near;

        // Discard any star that falls in the zone
        if (star_free_zone.contains(x, y))
        {
            ++i;
            continue;
        }

        // Else add it in the vector
        stars.push_back({ {x, y}, z });
    }

    // Depth ordering
    std::sort(stars.begin(), stars.end(), [](Star const& s_1, Star const& s_2) {
        return s_1.z > s_2.z;
        });

    return stars;
}