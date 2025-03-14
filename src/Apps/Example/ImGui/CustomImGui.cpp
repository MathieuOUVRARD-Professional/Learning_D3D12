#include <ImGui/CustomImGui.h>

void ImGuiPerfOverlay(bool open)
{
    static int location = 0;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (location >= 0)
    {
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    else if (location == -2)
    {
        // Center window
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Performance overlay", &open, window_flags))
    {
        ;
        ImGui::Text("Average perfs:");
        ImGui::Separator();
        ImGui::Text("%.1f FPS", io.Framerate);
        ImGui::Text("%.3f ms", 1000.0f / io.Framerate);
        
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom", NULL, location == -1)) location = -1;
            if (ImGui::MenuItem("Center", NULL, location == -2)) location = -2;
            if (ImGui::MenuItem("Top-left", NULL, location == 0)) location = 0;
            if (ImGui::MenuItem("Top-right", NULL, location == 1)) location = 1;
            if (ImGui::MenuItem("Bottom-left", NULL, location == 2)) location = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
            if (&open && ImGui::MenuItem("Close")) open = false;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

std::vector<float> ImGuiColorPicker(std::string* colorName, bool open)
{
    static ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    
    ImGuiColorEditFlags misc_flags =   ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_DisplayRGB;

    if (ImGui::Begin(colorName->c_str(), &open))
    {
        ImGui::SeparatorText("Color picker");        
        
        ImGuiColorEditFlags flags = misc_flags;
        flags |= ImGuiColorEditFlags_DisplayRGB;
        ImGui::ColorPicker4("MyColor##8", (float*)&color, flags, NULL);
    }
    ImGui::End();
    
    std::vector<float> toReturn = { color.x, color.y, color.z, color.w };
    return toReturn;
}

void InitGuizmo()
{
    ImGuizmo::SetOrthographic(false);

    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

    ImGuizmo::SetRect(0.0f, 0.0f, DXWindow::Get().GetWidth(), DXWindow::Get().GetHeigth());

    // Allow interaction only if ImGuizmo is being hovered
    if (ImGuizmo::IsOver())
    {
        spdlog::info("Gizmo is being hovered!\n");
    }
    if (ImGuizmo::IsUsing())
    {
        spdlog::info("Gizmo is being used!\n");
    }
}

void TranslateGuizmo(Camera& camera, glm::mat4& model)
{
   ImGuizmo::Manipulate(glm::value_ptr(camera.m_viewMatrix), glm::value_ptr(camera.m_projMatrix), ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD, glm::value_ptr(model));
}

void RotateGuizmo(Camera& camera, glm::mat4& model)
{
   ImGuizmo::Manipulate(glm::value_ptr(camera.m_viewMatrix), glm::value_ptr(camera.m_projMatrix), ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::WORLD, glm::value_ptr(model));
}

void ScaleGuizmo(Camera& camera, glm::mat4& model)
{
   ImGuizmo::Manipulate(glm::value_ptr(camera.m_viewMatrix), glm::value_ptr(camera.m_projMatrix), ImGuizmo::OPERATION::SCALE, ImGuizmo::MODE::WORLD, glm::value_ptr(model));
}

static bool translate = false;
static bool rotate = false;
static bool scale = false;

void TransformUI(Camera& camera, glm::mat4& model, MyTransform& transform)
{
    InitGuizmo();
    ImGui::Begin("ImGuizmo -- Settings");
    ImGui::Text("Operation:");
    ImGui::Checkbox("Translate", &translate);
    if (translate)
    {
        if (scale)
        {
            scale = false;
        }
        TranslateGuizmo(camera, model);
        transform.m_position = glm::vec3(model[3].x, model[3].y, model[3].z);
    }
    ImGui::SameLine();
    ImGui::Checkbox("Rotate", &rotate);
    if (rotate)
    {
        RotateGuizmo(camera, model);

    }
    ImGui::SameLine();
    ImGui::Checkbox("Scale", &scale);
    if (scale)
    {
        if (translate)
        {
            translate = false;
        }
        ScaleGuizmo(camera, model);
    }

    ImGui::Separator();

    // Translation sliders
    float modelPosition[] = { model[3].x, model[3].y, model[3].z };
    ImGui::Text("Position: ");
    if (ImGui::DragFloat3("##Position", modelPosition, 0.01f))
    {
        transform.m_position = glm::vec3(modelPosition[0], modelPosition[1], modelPosition[2]);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }

    // Rotation sliders
    float modelRotation[] = { transform.m_rotationEuler.x, transform.m_rotationEuler.y, transform.m_rotationEuler.z };
    ImGui::Text("Rotation: ");
    if (ImGui::DragFloat3("##Rotation", modelRotation, 0.5f, -181.0f, 181.0f))
    {
        if (modelRotation[0] > 180)
        {
            modelRotation[0] = -180;
        }
        if (modelRotation[1] > 180)
        {
            modelRotation[1] = -180;
        }
        if (modelRotation[2] > 180)
        {
            modelRotation[2] = -180;
        }
        if (modelRotation[0] < -180)
        {
            modelRotation[0] = 180;
        }
        if (modelRotation[1] < -180)
        {
            modelRotation[1] = 180;
        }
        if (modelRotation[2] < -180)
        {
            modelRotation[2] = 180;
        }
        glm::vec3 eulerRadians = glm::radians(glm::vec3(modelRotation[0], modelRotation[1], modelRotation[2]));
        transform.m_rotationEuler = glm::vec3(modelRotation[0], modelRotation[1], modelRotation[2]);

        // Apply new rotation
        transform.m_rotationMat = glm::mat4(1.0f);
        transform.m_rotationMat = glm::rotate(transform.m_rotationMat, eulerRadians.y, glm::vec3(0, 1, 0)); // Yaw
        transform.m_rotationMat = glm::rotate(transform.m_rotationMat, eulerRadians.x, glm::vec3(1, 0, 0)); // Pitch
        transform.m_rotationMat = glm::rotate(transform.m_rotationMat, eulerRadians.z, glm::vec3(0, 0, 1)); // Roll

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }

    // Scale sliders
    float modelScale[] = { glm::length(glm::vec3(model[0])), glm::length(glm::vec3(model[1])), glm::length(glm::vec3(model[2])) };
    ImGui::Text("Scale: ");
    if (ImGui::DragFloat3("##Scale", modelScale, 0.01f))
    {
        transform.m_scale = glm::vec3(modelScale[0], modelScale[1], modelScale[2]);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }

    ImGui::Separator();

    if (ImGui::Button("Reset position"))
    {
        transform.m_position = glm::vec3(10.0f, 25.0f, 10.0f);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Rotation"))
    {
        transform.m_rotationEuler = glm::vec3(0.0f, 0.0f, 0.0f);
        transform.m_rotationMat = glm::mat4(1.0f);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Scale"))
    {
        transform.m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }

    ImGui::End();
}

