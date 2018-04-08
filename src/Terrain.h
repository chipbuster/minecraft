#ifndef TERRAIN_H
#define TERRAIN_H

#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

// *** INDEXING CONVENTION *** //

/* All chunk functions return a single vector of values from low
   indices to high indices, varying quickly in x and slowly in z. Example:

   loc + (0, extent)                      loc + (extent, extent)
   +z    8        9        10       11
   |     4        5        6        7
   |     0        1        2        3
   loc    --->  +x                        loc + (extent, 0)

   This is the single-index indexing convention.

   For two indices, the first index varies with x and the second index varies
   with j.

   loc + (0, extent)                      loc + (extent, extent)
   +z    (0,2)   (1,2)    (2,2)    (3,2)
   |     (0,1)   (1,1)    (2,1)    (3,1)
   |     (0,0)   (1,0)    (2,0)    (3,0)
   loc    --->  +x                        loc + (extent, 0)

   This is the two-index indexing convention

*/

// A chunk is a finite-sized (16x16?) size of cubes
class Chunk {
    uint32_t tex_seed; // Seed used to determine textures
    uint32_t per_seed; // Seed used to determine heights (via perlin noise)

    std::vector<int> clampedEdges;

    public:
    Chunk(const glm::ivec2& location, int extent, std::mt19937& gen);

    std::vector<float> genPerlinNoise() const;
    std::vector<float> texSeedMap() const;

    glm::ivec2 loc;     // Coordinates of the bottom-left (x,z) corner
    int extent;        // Number of blocks in the x and z edges.
                       // both values should be powers of two.
};

//
class Terrain {
    std::unordered_map<glm::ivec2, Chunk, std::hash<glm::ivec2>,
                       std::equal_to<glm::ivec2>>
            chunkMap;
    std::mt19937 gen;
    int chunkExtent = 16;

    public:
    Terrain(uint64_t seed) : gen(seed) {}
    const Chunk& getChunk(glm::ivec2);

    std::vector<glm::vec3> chunkSurface(glm::ivec2 chunkCoords, glm::vec2 heights);
    glm::ivec2 getChunkCoords(glm::vec3 worldCoords) const;
    std::vector<glm::vec3> getOffsetsForRender(glm::vec3 camCoords, glm::vec2 heights);
    std::vector<float> getSeedsForRender(glm::vec3 camCoords);
};

#endif