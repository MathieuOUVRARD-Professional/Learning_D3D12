#pragma once

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include<string>
#include<d3d12.h>

class Light
{
	public:
        Light Directional(std::string name, glm::vec3 position = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f)), float intensity = 1.0f, glm::vec3 color = glm::vec3(1.0f));
        Light Point(std::string name, glm::vec3 position = glm::vec3(0.0f), float intensity = 1.0f, float radius = 100.0f, glm::vec3 color = glm::vec3(1.0f));
        Light Spot(std::string name, glm::vec3 position = glm::vec3(0.0f), glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f), float intensity = 1.0f, float radius = 100.0f, float innerAngle = 25.0f, float outerAngle = 30.0f, glm::vec3 colo = glm::vec3(1.0f));

        void ComputeViewProjMatrix(float ortoSize);

        void SendShaderParams(ID3D12GraphicsCommandList* cmdList, int bufferSlot);

        std::string m_name = "";

        uint32_t m_type = 0;                                                        // 0 = Directional, 1 = Point, 2 = Spot

        glm::vec3 m_position = glm::vec3(0.0f);                                     // World-space position (for point/spot lights)
        glm::vec3 m_direction = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));     // Normalized direction (for directional/spot lights)
        
        float m_intensity = 1.0f;                                                   // Light intensity (common for all types)        
        float m_radius = 100.0f;                                                    // For point/spot lights (max distance)
        float m_innerAngle = 25.0f;                                                 // Spot light cutoff (cosine of inner angle)
        float m_outerAngle = 30.0f;                                                  // Spot light outer cutoff (cosine of outer angle)

        glm::vec3 m_color = glm::vec3(1.0f);                                        // RGB light color              

        glm::mat4 m_viewProjMatrix = glm::mat4(1.0f);

	private:
        struct LightData
        {
            glm::vec3 position = glm::vec3(0.0f);
            uint32_t type = 0;


            glm::vec3 direction = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));
            float intensity = 1.0f;

            glm::vec3 color = glm::vec3(1.0f);
            float radius = 100.0f;

            float innerAngle = 25.0f;
            float outerAngle = 30.0f;
        };
};