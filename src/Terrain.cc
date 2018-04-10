#include "Terrain.h"
#include <cmath>
#include "glm/gtx/string_cast.hpp"

constexpr double pi = 3.14159265358979323846264338;

// Generate a point on a circle
glm::vec2 circleSample(float theta)
{
    return glm::vec2(cos(theta), sin(theta));
}

Chunk::Chunk(const glm::ivec2& location, int extent, std::mt19937& gen,
             Terrain* T)
{
    std::cout << "Location is " << glm::to_string(location) << std::endl;
    this->tex_seed = gen();
    this->loc = location;
    this->extent = extent;

    std::uniform_real_distribution<> dis(0.0,pi);

    for(int i = 0; i < 4; i++){
        O1grad[i] = circleSample(dis(gen));
    }
    for(int i = 0; i < 9; i++){
        O2grad[i] = circleSample(dis(gen));
    }

    if(location == glm::ivec2(0,0)){return;}

    // Rule: Chunk closest to chunk (0,0) takes precedence
    if(location.x == 0){ ((void)0);}
    else if(location.x > 0){ // Replace left edge
        const Chunk& C = T->getChunk(location + glm::ivec2(-1,0));
        O1grad[0] = C.O1grad[1];
        O1grad[2] = C.O1grad[3];
        O2grad[0] = C.O2grad[2];
        O2grad[3] = C.O2grad[5];
        O2grad[6] = C.O2grad[8];
    }
    else{ // Replace right edge
        const Chunk& C = T->getChunk(location + glm::ivec2(1,0));
        O1grad[1] = C.O1grad[0];
        O1grad[3] = C.O1grad[2];
        O2grad[2] = C.O2grad[0];
        O2grad[5] = C.O2grad[3];
        O2grad[8] = C.O2grad[6];
    }
    if(location.y == 0){ ((void)0); }
    else if(location.y > 0){ // Replace Bot edge
        const Chunk& C = T->getChunk(location + glm::ivec2(0,-1));
        O1grad[0] = C.O1grad[2];
        O1grad[1] = C.O1grad[3];
        O2grad[0] = C.O2grad[6];
        O2grad[1] = C.O2grad[7];
        O2grad[2] = C.O2grad[8];
    }
    else{
        const Chunk& C = T->getChunk(location + glm::ivec2(0,1));
        O1grad[2] = C.O1grad[0];
        O1grad[3] = C.O1grad[1];
        O2grad[6] = C.O2grad[0];
        O2grad[7] = C.O2grad[1];
        O2grad[8] = C.O2grad[2];
    }
}

constexpr float perlinFade(float t)
{
    return 6 * t * t * t * t * t - 15 * t * t * t * t + 10 * t * t * t;
}

/* coords: coordinates to take noise at. Scaled to be on a 0-1 square
   grads: gradients at corners. grad[0] is at (0,0), grad[1] is at (1,0),
                                grad[2] is at (0,1), grad[3] is at (1,1) */
float perlinNoiseSquare(const glm::vec2& coords, const glm::vec2* grad)
{
    glm::vec2 from0 = coords - glm::vec2(0, 0);
    glm::vec2 from1 = coords - glm::vec2(1, 0);
    glm::vec2 from2 = coords - glm::vec2(0, 1);
    glm::vec2 from3 = coords - glm::vec2(1, 1);

    float inf0 = glm::dot(grad[0], from0);
    float inf1 = glm::dot(grad[1], from1);
    float inf2 = glm::dot(grad[2], from2);
    float inf3 = glm::dot(grad[3], from3);

    float top = glm::mix(inf2, inf3, perlinFade(coords[0]));
    float bot = glm::mix(inf0, inf1, perlinFade(coords[0]));
    float mid = glm::mix(bot, top, perlinFade(coords[1]));

    return mid;
}



std::vector<float> Chunk::genPerlinNoise() const
{
    std::vector<float> outputs;
    outputs.resize(extent * extent);

    // TODO: rewrite to clamp to edges

    // Generate 16 equispaced points on a (0,2) square.
    glm::vec2 cellGrads1[4]; // Octave 1 noise
    float delta = 2 / float(extent - 1);
    for (int i = 0; i < extent; i++) {
        for (int j = 0; j < extent; j++) {
            // Set grads
            if (i < extent / 2 && j < extent / 2) {
                if (j < extent / 2) {
                    // lower-left quadrant
                    cellGrads1[0] = O2grad[0];
                    cellGrads1[1] = O2grad[1];
                    cellGrads1[2] = O2grad[3];
                    cellGrads1[3] = O2grad[4];
                } else {
                    // upper-left quadrant
                    cellGrads1[0] = O2grad[3];
                    cellGrads1[1] = O2grad[4];
                    cellGrads1[2] = O2grad[6];
                    cellGrads1[3] = O2grad[7];
                }
            } else {
                if (j < extent / 2) {
                    // lower-right quadrant
                    cellGrads1[0] = O2grad[1];
                    cellGrads1[1] = O2grad[2];
                    cellGrads1[2] = O2grad[4];
                    cellGrads1[3] = O2grad[5];

                } else {
                    // upper-right quadrant
                    cellGrads1[0] = O2grad[4];
                    cellGrads1[1] = O2grad[5];
                    cellGrads1[2] = O2grad[7];
                    cellGrads1[3] = O2grad[8];
                }
            }

            // Set coordinates
            float cellX = i * delta - (int)(i * delta);
            float cellY = j * delta - (int)(j * delta);
            glm::vec2 coordsLoOctave(cellX, cellY);
            glm::vec2 coordsHiOctave(i * delta / 2.0, j * delta / 2.0);

            int index = i + j * extent;

            outputs[index] =
                    0.25 * perlinNoiseSquare(coordsHiOctave, cellGrads1);
            outputs[index] += perlinNoiseSquare(coordsLoOctave, &O1grad[0]);
            outputs[index] /= 1.5;
        }
    }

    return outputs;
}

std::vector<float> Chunk::texSeedMap() const
{
    std::mt19937 gen(this->tex_seed);
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    std::vector<float> output;

    for (int i = 0; i < extent * extent; i++) {
        output.push_back(dis(gen));
    }
    return output;
}

const Chunk& Terrain::getChunk(glm::ivec2 chunkCoords)
{
    auto chunk = this->chunkMap.find(chunkCoords);
    if (chunk == this->chunkMap.end()) {
        Chunk c = Chunk(chunkCoords, this->chunkExtent, this->gen, this);
        auto status = this->chunkMap.insert({chunkCoords, c});
        if (!status.second) {
            std::cout << "Could not insert chunk into chunkMap" << std::endl;
        }
        return status.first->second;
    } else {
        return chunk->second;
    }
}

void fixNeighborGaps(std::vector<glm::vec3>& surfaceMap, int chunkExtent)
{
    for (int i = (int)surfaceMap.size() - 1; i >= 0; i--) {
        std::vector<int> neighbors = {i + 1, i - 1, i + chunkExtent,
                                      i - chunkExtent};

        for (int j : neighbors) {
            if (j < chunkExtent * chunkExtent && j > 0) {
                // This neighbor is in-bounds. How bad is the gap?
                float gapSize =
                        floor(surfaceMap[i].y - surfaceMap[j].y - 0.001);
                if (gapSize <= 0.0)
                    continue;
                // For each unit of gapSize, add a patch
                for (int k = 1; k <= gapSize; k++) {
                    surfaceMap.push_back(glm::vec3(surfaceMap[i].x,
                                                   surfaceMap[i].y - (float)k,
                                                   surfaceMap[i].z));
                }
            }
        }
    }
}

std::vector<glm::vec3> Terrain::chunkSurface(glm::ivec2 chunkCoords,
                                             glm::vec2 heights)
{
    const Chunk& C = this->getChunk(chunkCoords);
    std::vector<float> noise = C.genPerlinNoise();

    // Interpolate edge squares with neighbor's edge square to eliminate seaming
    for (int z = 0; z < 4; z++) {
        std::vector<float> neighborNoise;
        int start, Nstart;
        int stride, Nstride;

        switch (z) {
            case 0: // Bottom edge
                start = 0;
                Nstart = this->chunkExtent * this->chunkExtent -
                         this->chunkExtent;

                stride = 1;
                Nstride = 1;
                neighborNoise = this->getChunk(chunkCoords + glm::ivec2(0, -1))
                                        .genPerlinNoise();
                break;
            case 1: // Left edge
                start = 0;
                Nstart = this->chunkExtent - 1;
                stride = this->chunkExtent;
                Nstride = this->chunkExtent;
                neighborNoise = this->getChunk(chunkCoords + glm::ivec2(-1, 0))
                                        .genPerlinNoise();
                break;
            case 2: // Right Edge
                start = this->chunkExtent - 1;
                Nstart = 0;
                stride = this->chunkExtent;
                Nstride = this->chunkExtent;
                neighborNoise = this->getChunk(chunkCoords + glm::ivec2(1, 0))
                                        .genPerlinNoise();
                break;
            case 3: // Top edge
                start = this->chunkExtent * this->chunkExtent -
                        this->chunkExtent;
                stride = 1;
                Nstart = 0;
                Nstride = 1;
                neighborNoise = this->getChunk(chunkCoords + glm::ivec2(0, 1))
                                        .genPerlinNoise();
                break;
        }

        std::cout << "CASE " << z << std::endl;
        for (int i = 0; i < this->chunkExtent; i++) {
            std::cout << neighborNoise.size() << ", " << i*Nstride+Nstart << std::endl;
            noise[i * stride + start] =
                    glm::mix(noise[i * stride + start],
                             neighborNoise[i * Nstride + Nstart], 0.4);
        }
    }

    // Generate surface Map
    std::vector<glm::vec3> surfaceMap;
    surfaceMap.resize(this->chunkExtent * this->chunkExtent);
    float delta = heights.y - heights.x;

    for (int i = 0; i < this->chunkExtent; i++) {
        for (int j = 0; j < this->chunkExtent; j++) {
            int index = i + this->chunkExtent * j;
            glm::vec3 coords((float)C.loc.x + (float)i,
                             round(noise[index] * delta + heights.x),
                             (float)C.loc.y + (float)j);
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

std::vector<glm::vec3> Terrain::getOffsetsForRender(glm::vec3 camCoords,
                                                    glm::vec2 heights)
{
    glm::ivec2 center = this->getChunkCoords(camCoords);

    // Get surfacemap from each chunk
    std::vector<glm::vec3> offsets;
    int offsetSize = 5 * this->chunkExtent;
    offsets.resize(offsetSize * offsetSize);

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            glm::ivec2 c(center + glm::ivec2(i - 2, j - 2)); // Chunk's indices
            std::vector<glm::vec3> cOffsets = this->chunkSurface(c, heights);
            for (auto& offset : cOffsets) {
                offset += glm::vec3(c.x, 0.0, c.y) *
                          (float)(this->chunkExtent - 1);
            }

            // Map into offsets at the right locations
            for (int cj = 0; cj < this->chunkExtent; cj++) {
                for (int ci = 0; ci < this->chunkExtent; ci++) {
                    int ind = ci + i * this->chunkExtent 
                            + cj * offsetSize
                            + j * offsetSize * this->chunkExtent;
                    offsets[ind] = cOffsets[ci + this->chunkExtent * cj];
                }
            }
        }
    }

    // Fill seams
    fixNeighborGaps(offsets, offsetSize);

    // Sinkhole other cubes
    while (offsets.size() < 32000) {
        offsets.emplace_back(0.0f, -1000.0f, 0.0f);
    }

    return offsets;
}


std::vector<float> Terrain::getSeedsForRender(glm::vec3 camCoords){

    glm::ivec2 center = this->getChunkCoords(camCoords);
    std::vector<float> seeds;
    int offsetSize = 5 * this->chunkExtent;
    seeds.resize(offsetSize * offsetSize);

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            glm::ivec2 c(center + glm::ivec2(i - 2, j - 2)); // Chunk's indices
            std::vector<float> cSeeds = this->getChunk(c).texSeedMap();

            // Map into offsets at the right locations
            for (int cj = 0; cj < this->chunkExtent; cj++) {
                for (int ci = 0; ci < this->chunkExtent; ci++) {
                    int ind = ci + i * this->chunkExtent 
                            + cj * offsetSize
                            + j * offsetSize * this->chunkExtent;
                    seeds[ind] = cSeeds[ci + this->chunkExtent * cj];
                }
            }
        }
    }
    return seeds;
}