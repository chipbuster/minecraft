#ifndef MENGER_H
#define MENGER_H

#include <glm/glm.hpp>
#include <string>
#include <utility>
#include <vector>

using AABB = std::pair<glm::vec4, glm::vec4>;

class Menger {
    public:
    Menger();
    ~Menger();
    void set_nesting_level(int);
    bool is_dirty() const;
    void set_clean();
    void generate_geometry(std::vector<glm::vec4>& obj_vertices,
                           std::vector<glm::uvec3>& obj_faces) const;

    static std::string objdump(std::vector<glm::vec4>& obj_vertices,
                               std::vector<glm::uvec3>& obj_faces);

    const std::vector<glm::vec4>* getBaseVerts();
    const std::vector<glm::uvec3>* getBaseFaces();

    private:
    int nesting_level_ = 0;
    bool dirty_ = false;

    void gen_cubes(const AABB& bbox, std::vector<glm::vec4>& mainVerts,
                   std::vector<glm::uvec3>& mainFaces) const;

    void simplify(std::vector<glm::vec4>& mainVerts,
                  std::vector<glm::uvec3>& mainFaces) const;

    std::vector<AABB> subdivide_menger_endpoints(
            std::vector<AABB> endpts) const;

    void prettify(std::vector<glm::vec4>& obj_vertices,
                  std::vector<glm::uvec3>& faces, double distance_threshold);
};

glm::vec4 vec_gen(float x);
glm::vec4 vec_gen(float x, float y, float z);

#endif
