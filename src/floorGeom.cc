    std::vector<glm::vec4> floor_vertices;
    std::vector<glm::uvec3> floor_faces;
    {
        constexpr float m = 10.0f;
        constexpr float t = -2.0f;
        floor_vertices.push_back(glm::vec4(m, t, m, 1.0));
        floor_vertices.push_back(glm::vec4(-m, t, m, 1.0));
        floor_vertices.push_back(glm::vec4(-m, t, -m, 1.0));
        floor_vertices.push_back(glm::vec4(m, t, -m, 1.0));
        floor_faces.push_back(glm::uvec3(0, 2, 1));
        floor_faces.push_back(glm::uvec3(3, 2, 0));
    }

    std::vector<glm::vec4> ocean_vertices;
    std::vector<glm::uvec4> ocean_faces;

    {
        constexpr float m = 20.0f;
        constexpr float t = -2.0f;
        constexpr int chunks = 16;
        constexpr float delta = (2*m) / (chunks);
        for(int i = 0; i <= (chunks); i++){
            for(int j = 0; j <= (chunks); j++){
                ocean_vertices.push_back(
                    glm::vec4(-m + i * delta, t, -m + j * delta, 1.0));
            }
        }

        for(int i = 0; i < chunks; i++){
            for(int j = 0; j < chunks; j++){
                ocean_faces.push_back(glm::uvec4(i * (chunks + 1) + j
                                     , (i + 1) * (chunks + 1) + j
                                     , (i + 1) * (chunks + 1) + j + 1
                                     , i * (chunks + 1) + j + 1));
            }
        }
        /*
        for(const auto& vert : ocean_vertices){
            std::cout << vert << " ";
        }
        std::cout << std::endl;

        for(const auto& f : ocean_faces){
            std::cout << f << " ";
        }
        std::cout << std::endl;
        */

    }