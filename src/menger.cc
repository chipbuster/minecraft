#include "menger.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

namespace {
const int kMinLevel = 0;
const int kMaxLevel = 4;
}; // namespace

using std::cout;
using std::endl;

std::ostream& operator<<(std::ostream& os, const glm::vec4 x)
{
    os << "[" << x[0] << "," << x[1] << "," << x[2] << "," << x[3] << "]";
    return os;
}
std::ostream& operator<<(std::ostream& os, const glm::vec3 x)
{
    os << "[" << x[0] << "," << x[1] << "," << x[2] << "]";
    return os;
}

const std::vector<glm::vec4> baseVerts = {
        glm::vec4(0.0, 0.0, 0.0, 1.0), glm::vec4(1.0, 0.0, 0.0, 1.0),
        glm::vec4(0.0, 1.0, 0.0, 1.0), glm::vec4(0.0, 0.0, 1.0, 1.0),
        glm::vec4(1.0, 1.0, 0.0, 1.0), glm::vec4(0.0, 1.0, 1.0, 1.0),
        glm::vec4(1.0, 0.0, 1.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0)};

const std::vector<glm::uvec3> baseFaces = {glm::uvec3(3, 6, 5) // +Z face
                                           ,
                                           glm::uvec3(7, 5, 6),
                                           glm::uvec3(0, 2, 1) // -Z face
                                           ,
                                           glm::uvec3(2, 4, 1),
                                           glm::uvec3(4, 2, 7) // +Y face
                                           ,
                                           glm::uvec3(5, 7, 2),
                                           glm::uvec3(1, 3, 0) // -Y face
                                           ,
                                           glm::uvec3(1, 6, 3),
                                           glm::uvec3(1, 4, 6) // +X face
                                           ,
                                           glm::uvec3(4, 7, 6),
                                           glm::uvec3(2, 0, 5) // -X face
                                           ,
                                           glm::uvec3(0, 3, 5)};

Menger::Menger()
{
    // Add additional initialization if you like
}

Menger::~Menger() {}

void Menger::set_nesting_level(int level)
{
    nesting_level_ = level;
    dirty_ = true;
}

bool Menger::is_dirty() const
{
    return dirty_;
}

void Menger::set_clean()
{
    dirty_ = false;
}

/* Generate a cube in the given endpoints */
void Menger::gen_cubes(const AABB& bbox, std::vector<glm::vec4>& mainVerts,
                       std::vector<glm::uvec3>& mainFaces) const
{
    auto e1 = bbox.first;
    auto e2 = bbox.second;
    size_t numVerts = mainVerts.size();

    std::vector<glm::vec4> verts(baseVerts);
    std::vector<glm::uvec3> faces(baseFaces);

    glm::vec4 scaling = glm::abs(e2 - e1);
    scaling[3] = 1.0;

    for (auto& vert : verts) {
        vert = vert * scaling;
        vert = e1 + vert;
        vert[3] = 1.0;
    }
    for (auto& face : faces) {
        face += glm::uvec3(numVerts);
    }

    mainVerts.insert(mainVerts.end(), verts.begin(), verts.end());
    mainFaces.insert(mainFaces.end(), faces.begin(), faces.end());

    // Sanity check our renumbering
    for (int i = 0; i < 8; i++) {
        assert(verts[i] == mainVerts[i + numVerts]);
    }
}

/* Given a set of boxes, subdivide into additional menger endpoints */
std::vector<AABB> Menger::subdivide_menger_endpoints(
        std::vector<AABB> endpts) const
{
    std::vector<AABB> subdivs;
    subdivs.reserve(endpts.size() * 20);
    for (const auto& ep : endpts) {
        const glm::vec4& e1 = ep.first;
        const glm::vec4& e2 = ep.second;
        glm::vec4 deltas = (e2 - e1);
        deltas *= (1.0 / 3.0);

        // Subdivide cube into a 3x3x3, skipping the the core 7 cubes, i.e. the
        // "solid core" of a Rubik's cube. Hardcode because elegance is for
        // chumps.
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                for (int k = 0; k < 3; k++) {
                    glm::vec4 split_e1 = glm::vec4(i, j, k, 0.0) * deltas + e1;
                    glm::vec4 split_e2 = split_e1 + deltas;
                    split_e1[3] = 1.0;
                    split_e2[3] = 1.0;

                    // Hardcode skip conditions
                    if ((i == 1 && j == 1 && k == 0) ||
                        (i == 0 && j == 1 && k == 1) ||
                        (i == 1 && j == 0 && k == 1) ||
                        (i == 1 && j == 1 && k == 1) || // Center cube
                        (i == 1 && j == 1 && k == 2) ||
                        (i == 2 && j == 1 && k == 1) ||
                        (i == 1 && j == 2 && k == 1)) {
                        continue; // Do not add cube
                    } else {
                        subdivs.push_back(std::make_pair(split_e1, split_e2));
                    }
                }
            }
        }
    }
    return subdivs;
}

// Unify all copies of obj_vertices,
void Menger::prettify(std::vector<glm::vec4>& obj_vertices,
                      std::vector<glm::uvec3>& faces, double distance_threshold)
{
    // NOOP
}

void Menger::generate_geometry(std::vector<glm::vec4>& obj_vertices,
                               std::vector<glm::uvec3>& obj_faces) const
{
    std::cout << "Generating Menger Geometry at level " << nesting_level_
              << ", please be patient..." << std::flush;
    obj_vertices.clear();
    obj_faces.clear();

    // First generate the correct set of subcubes
    AABB mainBox = std::make_pair(vec_gen(-0.5), vec_gen(0.5));
    int ctr = nesting_level_;
    std::vector<AABB> endPts;
    endPts.push_back(mainBox);
    std::vector<AABB> newEndPts;
    while (ctr > 0) {
        newEndPts = this->subdivide_menger_endpoints(endPts);
        endPts = newEndPts; // The compiler can optimize this out, right?
        ctr--;
    }

    // Next, for each subcube, generate appropriate geometry
    for (const AABB& box : endPts) {
        this->gen_cubes(box, obj_vertices, obj_faces);
    }

    // Finally (optional): simplify structure
    ((void)0);

    std::cout << "Finished!" << std::endl;
}

glm::vec4 vec_gen(float x)
{
    glm::vec4 z(x);
    z[3] = 1.0;
    return z;
}
glm::vec4 vec_gen(float x, float y, float z)
{
    return glm::vec4(x, y, z, 1.0);
}

std::string Menger::objdump(std::vector<glm::vec4>& obj_vertices,
                            std::vector<glm::uvec3>& obj_faces)
{
    std::ostringstream ss; // Build output in this stringstream

    for(const auto& vert : obj_vertices){
        ss << "v " << vert[0] << " " << vert[1] << " " << vert[2] << '\n';
    }
    for(const auto& face : obj_faces){
        ss << "f " << face[0] + 1 << " " << face[1] + 1 << " " << face[2] + 1 
           << '\n';
    }
    return ss.str();
}