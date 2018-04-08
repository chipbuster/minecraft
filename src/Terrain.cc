#include "Terrain.h"
#include <cmath>
#include "glm/gtx/string_cast.hpp"

constexpr double pi = 3.14159265358979323846264338;

Chunk::Chunk(const glm::ivec2& location, int extent, std::mt19937& gen)
{
    this->tex_seed = gen();
    this->per_seed = gen();
    this->loc = location;
    this->extent = extent;
}

constexpr float perlinFade(float t)
{
    return 6 * t * t * t * t * t - 15 * t * t * t * t + 10 * t * t * t;
}

/* coords: coordinates to take noise at. Scaled to be on a 0-1 square
   grads: gradients at corners. grad[0] is at (0,0), grad[1] is at (1,0),
                                grad[2] is at (0,1), grad[3] is at (1,1) */
float perlinNoiseSquare(const glm::vec2& coords, glm::vec2* grad)
{
    glm::vec2 from0 = coords - glm::vec2(0, 0);
    glm::vec2 from1 = coords - glm::vec2(1, 0);
    glm::vec2 from2 = coords - glm::vec2(0, 1);
    glm::vec2 from3 = coords - glm::vec2(1, 1);

    float inf0 = glm::dot(grad[0], from0);
    float inf1 = glm::dot(grad[1], from1);
    float inf2 = glm::dot(grad[2], from2);
    float inf3 = glm::dot(grad[3], from3);

    float top = glm::mix(inf2, inf3, coords[0]);
    float bot = glm::mix(inf0, inf1, coords[0]);
    float mid = glm::mix(bot, top, coords[1]);

    return mid;
}

// Generate a point on a circle
glm::vec2 circleSample(float theta)
{
    return glm::vec2(cos(theta), sin(theta));
}

std::vector<float> Chunk::genPerlinNoise(uint64_t seed) const
{
    std::mt19937 ran(seed);
    std::vector<float> outputs;
    outputs.resize(extent * extent);

    // Generate gradients at the corners, edge midpoints, and center
    // Follow row-major bottom-left to top-right convention (see Terrain.h)
    glm::vec2 chunkGrads3[9];
    for (int i = 0; i < 9; i++) {
        chunkGrads3[i] = circleSample(ran());
    }

    // Generate a higher octave at the corners only
    glm::vec2 chunkGrads2[4];
    for (int i = 0; i < 9; i++) {
        chunkGrads2[i] = circleSample(ran());
    }

    // Generate 16 equispaced points on a (0,2) square.
    glm::vec2 cellGrads1[4]; // Octave 1 noise
    float delta = 2 / float(extent - 1);
    for (int i = 0; i < extent; i++) {
        for (int j = 0; j < extent; j++) {
            // Set grads
            if (i < extent / 2) {
                if (j < extent / 2) {
                    // lower-left quadrant
                    cellGrads1[0] = chunkGrads3[0];
                    cellGrads1[1] = chunkGrads3[1];
                    cellGrads1[2] = chunkGrads3[3];
                    cellGrads1[3] = chunkGrads3[4];
                } else {
                    // upper-left quadrant
                    cellGrads1[0] = chunkGrads3[3];
                    cellGrads1[1] = chunkGrads3[4];
                    cellGrads1[2] = chunkGrads3[6];
                    cellGrads1[3] = chunkGrads3[7];
                }
            } else {
                if (j < extent / 2) {
                    // lower-right quadrant
                    cellGrads1[0] = chunkGrads3[1];
                    cellGrads1[1] = chunkGrads3[2];
                    cellGrads1[2] = chunkGrads3[4];
                    cellGrads1[3] = chunkGrads3[5];

                } else {
                    // upper-right quadrant
                    cellGrads1[0] = chunkGrads3[4];
                    cellGrads1[1] = chunkGrads3[5];
                    cellGrads1[2] = chunkGrads3[7];
                    cellGrads1[3] = chunkGrads3[8];
                }
            }

            // Set coordinates
            float cellX = i * delta - (int)(i * delta);
            float cellY = j * delta - (int)(j * delta);
            glm::vec2 coordsLoOctave(cellX, cellY);
            glm::vec2 coordsHiOctave(i * delta / 2.0, j * delta / 2.0);

            int index = i + j * extent;

            outputs[index] =
                    0.5 * perlinNoiseSquare(coordsHiOctave, cellGrads1);
            outputs[index] += perlinNoiseSquare(coordsLoOctave, chunkGrads2);
            outputs[index] /= 1.5;
        }
    }

    return outputs;
}

std::vector<float> Chunk::heightMap(float minHeight, float maxHeight) const
{
    float delta = maxHeight - minHeight;
    std::vector<float> noise = this->genPerlinNoise(this->per_seed);

    for (auto& elem : noise) {
        elem = delta * elem + minHeight;
    }
    return noise;
}

std::vector<uint32_t> Chunk::texSeedMap() const
{
    std::mt19937 gen(this->tex_seed);
    std::vector<uint32_t> output;

    for (int i = 0; i < extent * extent; i++) {
        output.push_back(gen());
    }
    return output;
}

const Chunk& Terrain::getChunk(glm::ivec2 chunkCoords)
{
    auto chunk = this->chunkMap.find(chunkCoords);
    if (chunk == this->chunkMap.end()) {
        Chunk c = Chunk(chunkCoords, this->chunkExtent, this->gen);
        auto status = this->chunkMap.insert({chunkCoords, c});
        if (!status.second) {
            std::cout << "Could not insert chunk into chunkMap" << std::endl;
        }
        return status.first->second;
    } else {
        return chunk->second;
    }
}

std::vector<glm::vec3> Terrain::chunkSurface(glm::ivec2 chunkCoords,
                                             glm::vec2 heights)
{
    const Chunk& C = this->getChunk(chunkCoords);
    std::vector<float> heightMap = C.heightMap(heights.x, heights.y);
    std::vector<glm::vec3> surfaceMap; surfaceMap.resize(this->chunkExtent * this->chunkExtent);

    // TODO
    for (int i = 0; i < this->chunkExtent; i++) {
        for (int j = 0; j < this->chunkExtent; j++) {
            int index = i + this->chunkExtent * j;
            glm::vec3 coords((float)C.loc.x + (float)i
                            , round(heightMap[index])
                            , (float)C.loc.y + (float)j);
            surfaceMap[index] = coords;
        }
    }

    return surfaceMap;
}

// Given world coordinates, return which chunk they are over
glm::ivec2 Terrain::getChunkCoords(glm::vec3 coords) const
{
    return glm::ivec2((int)coords.x / this->chunkExtent,
                      (int)coords.z / this->chunkExtent);
}

std::vector<glm::vec3> Terrain::getOffsetsForRender(glm::vec3 camCoords, glm::vec2 heights)
{
    glm::ivec2 center = this->getChunkCoords(camCoords);

    // Get visible chunks: a 5x5 set centered on the camera
    std::vector<glm::ivec2> chunkCoords;
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            chunkCoords.push_back(center + glm::ivec2(i, j));
        }
    }

    // Get surfacemap from each chunk
    std::vector<glm::vec3> offsets;
    offsets.reserve((5 * this->chunkExtent) * (5 * this->chunkExtent));

    for(const auto& c : chunkCoords){
        std::vector<glm::vec3> cOffsets = this->chunkSurface(c, heights);
        for(auto& offset : cOffsets){
            offset += glm::vec3(c.x, 0.0, c.y) * (float)(this->chunkExtent - 1);
        }
        offsets.insert(offsets.end(), cOffsets.begin(), cOffsets.end());
    }
    return offsets;
}