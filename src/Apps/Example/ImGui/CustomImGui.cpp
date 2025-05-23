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

glm::vec3 ImGuiColorPicker(std::string name)
{
    static ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    
    ImGuiColorEditFlags misc_flags =   ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_DisplayRGB;

	ImGui::SeparatorText(name.c_str());
	ImGuiColorEditFlags flags = misc_flags;
	flags |= ImGuiColorEditFlags_DisplayRGB;
	ImGui::ColorPicker4("MyColor##8", (float*)&color, flags, NULL);    
    
    glm::vec3 toReturn = glm::vec3(color.x, color.y, color.z);
    return toReturn;
}

void InitGuizmo()
{
    ImGuizmo::SetOrthographic(false);

    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

    ImGuizmo::SetRect(0.0f, 0.0f, (float)DXWindow::Get().GetWidth(), (float)DXWindow::Get().GetHeigth());

    // Allow interaction only if ImGuizmo is being hovered
    if (ImGuizmo::IsOver())
    {

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

static bool translate = true;
static bool rotate = false;
static bool scale = false;
static bool rotating = false;
static bool cursorWarping = false;
static bool counterWrap = false;
static POINT lastMousePos = POINT();
static float lastPosition[3] = { 0.0f, 0.0f, 0.0f };
static float lastRotation[3] = { 0.0f, 0.0f, 0.0f };
static float lastScale[3] = { 0.0f, 0.0f, 0.0f };
static float activeAxis = -1;
static float warpOffsetX = 0;;

void TransformUI(Camera& camera, glm::mat4& model, MyTransform& transform)
{
    InitGuizmo();
    ImGui::Begin("Transform", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);    
    ImGui::SetWindowSize(ImVec2(350.0f, ImGui::GetWindowSize().y));
    ImGui::SetWindowPos(ImVec2(DXWindow::Get().GetWidth() - ImGui::GetWindowSize().x, 0.0f));

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
        if (ImGuizmo::IsUsing())
        {           
            rotating = true;
        }
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
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 15.0f);
    if (ImGui::DragFloat3("##Position", modelPosition, 0.01f))
    {
        // Detect which axis is being modified by comparing old and new values
        if (modelPosition[0] != lastPosition[0])
        {
            activeAxis = 0;
        }
        else if (modelPosition[1] != lastPosition[1])
        {
            activeAxis = 1;
        }
        else if (modelPosition[2] != lastPosition[2])
        {
            activeAxis = 2;
        }

        POINT pt = POINT();
        GetCursorPos(&pt);
        if (counterWrap)
        {
            // Compute the movement offset caused by warping and subtract it only on the active axis
            for (int i = 0; i < 3; i++)
            {
                if (activeAxis == i)
                {
                    modelPosition[i] -= warpOffsetX * 0.01f; // Counteract the warp movement
                }
            }
            counterWrap = false;
        }

        if (cursorWarping)
        {
            cursorWarping = false; // Reset warp flag
            counterWrap = true;
            warpOffsetX =(float) (pt.x - lastMousePos.x);
        }
        else
        {
            if (pt.x <= 1.0f)
            {
                cursorWarping = true;
                SetCursorPos(DXWindow::Get().GetWidth() - 1, (int)pt.y);
            }
            else if (pt.x >= DXWindow::Get().GetWidth() - 1.0f)
            {
                cursorWarping = true;
                SetCursorPos(1, (int)pt.y);
            }
        }

        lastMousePos = pt;
        for (int i = 0; i < 3; i++)
        {
            lastPosition[i] = modelPosition[i];
        }

        transform.m_position = glm::vec3(modelPosition[0], modelPosition[1], modelPosition[2]);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }

    if (rotating)
    {
        transform.m_rotationEuler = glm::degrees(glm::eulerAngles(glm::quat_cast(glm::mat3(model))));
        rotating = false;
    }

    // Rotation sliders
    float modelRotation[] = { transform.m_rotationEuler.x, transform.m_rotationEuler.y, transform.m_rotationEuler.z };
    ImGui::Text("Rotation: ");
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 15.0f);
    if (ImGui::DragFloat3("##Rotation", modelRotation, 0.5f))
    {
        // Detect which axis is being modified by comparing old and new values
        if (modelRotation[0] != lastRotation[0])
        {
            activeAxis = 0;
        }
        else if (modelRotation[1] != lastRotation[1])
        {
            activeAxis = 1;
        }
        else if (modelRotation[2] != lastRotation[2])
        {
            activeAxis = 2;
        }        

        POINT pt = POINT();
        GetCursorPos(&pt);
        if (counterWrap)
        {
            // Compute the movement offset caused by warping and subtract it only on the active axis
            for (int i = 0; i < 3; i++)
            {
                if (activeAxis == i)
                {
                    modelRotation[i] -= warpOffsetX * 0.5f; // Counteract the warp movement
                }
            }
            counterWrap = false;
        }

        if (cursorWarping)
        {
            cursorWarping = false; // Reset warp flag
            counterWrap = true;
            warpOffsetX = (float)(pt.x - lastMousePos.x);
        }
        else
        {
            if (pt.x <= 1.0f)
            {
                cursorWarping = true;
                SetCursorPos(DXWindow::Get().GetWidth() - 1, (int)pt.y);
            }
            else if (pt.x >= DXWindow::Get().GetWidth() - 1.0f)
            {
                cursorWarping = true;
                SetCursorPos(1, (int)pt.y);
            }
        }

        lastMousePos = pt;
        for (int i = 0; i < 3; i++)
        {
            lastRotation[i] = modelRotation[i];
        }

        for (int i = 0; i < 3; i++) {
            if (modelRotation[i] > 180)
            {
                modelRotation[i] = -180;
            }

            if (modelRotation[i] < -180)
            {
                modelRotation[i] = 180;
            }
        }

        glm::vec3 eulerRadians = glm::radians(glm::vec3(modelRotation[0], modelRotation[1], modelRotation[2]));
        transform.m_rotationEuler = glm::vec3(modelRotation[0], modelRotation[1], modelRotation[2]);

        glm::quat rotationQuat = glm::quat(eulerRadians);
        transform.m_rotationMat = glm::toMat4(rotationQuat);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }

    // Scale sliders
    float modelScale[] = { glm::length(glm::vec3(model[0])), glm::length(glm::vec3(model[1])), glm::length(glm::vec3(model[2])) };
    ImGui::Text("Scale: ");
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 15.0f);
    if (ImGui::DragFloat3("##Scale", modelScale, 0.01f))
    {
        // Detect which axis is being modified by comparing old and new values
        if (modelScale[0] != lastScale[0])
        {
            activeAxis = 0;
        }
        else if (modelScale[1] != lastScale[1])
        {
            activeAxis = 1;
        }
        else if (modelScale[2] != lastScale[2])
        {
            activeAxis = 2;
        }

        POINT pt = POINT();
        GetCursorPos(&pt);
        if (counterWrap)
        {           
            // Compute the movement offset caused by warping and subtract it only on the active axis
            for (int i = 0; i < 3; i++)
            {
                if (activeAxis == i)
                {
                    modelScale[i] -= warpOffsetX * 0.01f; // Counteract the warp movement
                }
            }
            counterWrap = false;
        }

        if (cursorWarping)
        {
            cursorWarping = false; // Reset warp flag
            counterWrap = true;
            warpOffsetX = (float)(pt.x - lastMousePos.x);
        }
        else
        {
            if (pt.x <= 1.0f)
            {
                cursorWarping = true;
                SetCursorPos(DXWindow::Get().GetWidth() - 1, (int)pt.y);
            }
            else if (pt.x >= DXWindow::Get().GetWidth() - 1.0f)
            {
                cursorWarping = true;
                SetCursorPos(1, (int)pt.y);
            }
        }

        lastMousePos = pt;
        for (int i = 0; i < 3; i++)
        {
            lastScale[i] = modelScale[i];
        }

        transform.m_scale = glm::vec3(modelScale[0], modelScale[1], modelScale[2]);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }

    ImGui::Separator();

    if (ImGui::Button("Reset Position", ImVec2((ImGui::GetWindowWidth() - 30.0f) / 3.0f, 0.0f)))
    {
        transform.m_position = glm::vec3(10.0f, 25.0f, 10.0f);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Rotation", ImVec2((ImGui::GetWindowWidth() - 30.0f) / 3.0f, 0.0f)))
    {
        transform.m_rotationEuler = glm::vec3(0.0f, 0.0f, 0.0f);
        transform.m_rotationMat = glm::mat4(1.0f);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Scale", ImVec2((ImGui::GetWindowWidth() - 30.0f)/3.0f, 0.0f)))
    {
        transform.m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

        model = glm::translate(glm::mat4(1.0f), transform.m_position);
        model *= transform.m_rotationMat;
        model = glm::scale(model, transform.m_scale);
    }

    ImGui::End();
}

void ImageFromResource(ID3D12Resource* resource, ExampleDescriptorHeapAllocator& heapAllocator, ImVec2 size)
{

    D3D12_CPU_DESCRIPTOR_HANDLE imgui_cpu_handle;
    D3D12_GPU_DESCRIPTOR_HANDLE imgui_gpu_handle;

    std::map<ID3D12Resource*, UINT64>::iterator it;
    it = imguiTexturesHandleMap.find(resource);

    if (it == imguiTexturesHandleMap.end())
    {
        heapAllocator.Alloc(&imgui_cpu_handle, &imgui_gpu_handle); // Allocate a descriptor

        imguiTexturesHandleMap.emplace(resource, imgui_gpu_handle.ptr);

        D3D12_SHADER_RESOURCE_VIEW_DESC rvd = {};

        rvd.Format = resource->GetDesc().Format == DXGI_FORMAT_D32_FLOAT ? DXGI_FORMAT_R32_FLOAT : resource->GetDesc().Format;
        rvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        rvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        rvd.Texture2D.MipLevels = 1;

        DXContext::Get().GetDevice()->CreateShaderResourceView(resource, &rvd, imgui_cpu_handle);

    }
    else
    {
        imgui_gpu_handle.ptr = it->second;
    }  

    if (size.x == 0.0f && size.y == 0.0f)
    {
        size.x = ImGui::GetWindowWidth() - 15.0f;
        size.y = ImGui::GetWindowWidth() - 15.0f;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::Image((ImTextureID)imgui_gpu_handle.ptr, size);
    if (ImGui::BeginItemTooltip())
    {
        float region_sz = 32.0f;
        float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
        float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
        float zoom = 4.0f;
        if (region_x < 0.0f) { region_x = 0.0f; }
        else if (region_x > size.x - region_sz) { region_x = size.y - region_sz; }
        if (region_y < 0.0f) { region_y = 0.0f; }
        else if (region_y > size.x - region_sz) { region_y = size.y - region_sz; }
        //ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
        //ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
        ImVec2 uv0 = ImVec2((region_x) / size.x, (region_y) / size.y);
        ImVec2 uv1 = ImVec2((region_x + region_sz) / size.x, (region_y + region_sz) / size.y);
        ImGui::Image((ImTextureID)imgui_gpu_handle.ptr, ImVec2(region_sz * zoom * 1.5f, region_sz * zoom * 1.5f), uv0, uv1);
        ImGui::EndTooltip();
    }
}

ImGuiID popupID = 0;
static int item_selected_idx = 0;
static bool colorPickerIsUsed = false;
void LightInterface(std::vector<Light*>& lights)
{
    ImGui::Begin("Lights", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowSize(ImVec2(350.0f, 300.0f));
    ImGui::SetWindowPos(ImVec2(DXWindow::Get().GetWidth() - ImGui::GetWindowSize().x, 225.0f));

    const char* lightTypes[] = { "Directional", "Point", "Spot" };

    for (Light*& light : lights)
    {
        if(ImGui::CollapsingHeader(light->m_name.c_str()))
        {
            ImGui::Indent(10.0f);             

			// Type
			ImGui::Text("Type:");
            std::string type = lightTypes[light->m_type];
            if (ImGui::BeginCombo(("##" + light->m_name + "_TYPE").c_str(), type.c_str()))
            {
                for (int i = 0; i < 3; i++)
                {
                    const bool is_selected = item_selected_idx == i;
                    if (ImGui::Selectable(((std::string)lightTypes[i] + "##" + light->m_name + "_TYPE").c_str(), is_selected))
                    {
                        item_selected_idx = i;

                        light->m_type = i;
                        light->m_direction = glm::normalize(-light->m_position);
                        light->m_radius = i == 0 ? 100.0f : 20.0f;
                        light->m_intensity = 1.0f;
                        light->m_innerAngle = 25.0f;
                        light->m_outerAngle = 30.0f;
                    }

                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            //Color
            ImGui::Text("Color:");
            ImVec4 color = ImVec4(light->m_color.x, light->m_color.y, light->m_color.z, 1.0f);
            if (ImGui::ColorButton(("##current" + light->m_name + "_COLOR").c_str(), color))
            {
				if (!colorPickerIsUsed) 
                {
                    colorPickerIsUsed = true;
					ImGui::OpenPopup("LightColor");
				}                
            }
			if (ImGui::BeginPopup("LightColor", 0))
			{
				light->m_color = ImGuiColorPicker(light->m_name + " color");

				ImGui::EndPopup();
                colorPickerIsUsed = false;
			}

            // Intensity
			ImGui::Text("Intensity:");
			float intensity = light->m_intensity;
			if (ImGui::DragFloat(("##" + light->m_name + "_INTENSITY").c_str(), &intensity, 0.001f, 0, 2.0f, "%.3f"))
			{
				light->m_intensity = intensity;
			}

            // Range
            bool isDirect = light->m_type == 0;
            ImGui::BeginDisabled(isDirect);
			ImGui::Text("Range:");
			float radius = light->m_radius;
			if (ImGui::DragFloat(("##" + light->m_name + "_RADIUS").c_str(), &radius, 0.1f, 0, 1000.0f, "%.2f"))
			{
				light->m_radius = radius;
			}
            ImGui::EndDisabled();

            //Spot Angles
            bool isSpot = light->m_type == 2;
            ImGui::BeginDisabled(!isSpot);
			ImGui::Text("Spot inner angle:");
			float innerAngle = light->m_innerAngle;
			if (ImGui::DragFloat(("##" + light->m_name + "_INNER_ANGLE").c_str(), &innerAngle, 0.1f, 0, 180.0f, "%.2f"))
			{
				light->m_innerAngle = innerAngle;
			}

			ImGui::Text("Spot outer angle:");
			float m_outerAngle = light->m_outerAngle;
			if (ImGui::DragFloat(("##" + light->m_name + "_OUTER_ANGLE").c_str(), &m_outerAngle, 0.1f, 0, 180.0f, "%.2f"))
			{
				light->m_outerAngle = m_outerAngle;
			}
			ImGui::EndDisabled();            
        }
    }

    ImGui::End();
}

