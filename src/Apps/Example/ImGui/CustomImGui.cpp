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
//
//void EditTransform(const Camera& camera, glm::mat4& matrix)
//{
//    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
//    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
//    if (ImGui::IsKeyPressed(90))
//        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
//    if (ImGui::IsKeyPressed(69))
//        mCurrentGizmoOperation = ImGuizmo::ROTATE;
//    if (ImGui::IsKeyPressed(82)) // r Key
//        mCurrentGizmoOperation = ImGuizmo::SCALE;
//    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
//        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
//    ImGui::SameLine();
//    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
//        mCurrentGizmoOperation = ImGuizmo::ROTATE;
//    ImGui::SameLine();
//    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
//        mCurrentGizmoOperation = ImGuizmo::SCALE;
//    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
//    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(matrix), matrixTranslation, matrixRotation, matrixScale);
//    ImGui::InputFloat3("Tr", matrixTranslation,"%.3f", 3);
//    ImGui::InputFloat3("Rt", matrixRotation, "%.3f", 3);
//    ImGui::InputFloat3("Sc", matrixScale, "%.3f", 3);
//    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, glm::value_ptr(matrix));
//
//    if (mCurrentGizmoOperation != ImGuizmo::SCALE)
//    {
//        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
//            mCurrentGizmoMode = ImGuizmo::LOCAL;
//        ImGui::SameLine();
//        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
//            mCurrentGizmoMode = ImGuizmo::WORLD;
//    }
//    static bool useSnap(false);
//    if (ImGui::IsKeyPressed(83))
//        useSnap = !useSnap;
//    ImGui::Checkbox("", &useSnap);
//    ImGui::SameLine();
//    vec_t snap;
//    switch (mCurrentGizmoOperation)
//    {
//    case ImGuizmo::TRANSLATE:
//        snap = config.mSnapTranslation;
//        ImGui::InputFloat3("Snap", &snap.x);
//        break;
//    case ImGuizmo::ROTATE:
//        snap = config.mSnapRotation;
//        ImGui::InputFloat("Angle Snap", &snap.x);
//        break;
//    case ImGuizmo::SCALE:
//        snap = config.mSnapScale;
//        ImGui::InputFloat("Scale Snap", &snap.x);
//        break;
//    }
//    ImGuiIO& io = ImGui::GetIO();
//    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
//    ImGuizmo::Manipulate(camera.mView.m16, camera.mProjection.m16, mCurrentGizmoOperation, mCurrentGizmoMode, matrix.m16, NULL, useSnap ? &snap.x : NULL);
//}


//if (ImGui::IsMousePosValid())
//ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
//else
//ImGui::Text("Mouse Position: <invalid>");