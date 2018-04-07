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

namespace CubeData{

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

}