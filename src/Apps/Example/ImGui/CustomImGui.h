#include"imgui.h"
#include"ImGuizmo.h"

#include<Support/Window.h>
#include<Support/Camera.h>
#include<Support/Transform.h>

#include<vector>
#include<string>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

void ImGuiPerfOverlay(bool open);
std::vector<float> ImGuiColorPicker(std::string* colorName, bool open);
void InitGuizmo();
void TranslateGuizmo(Camera& camera, glm::mat4& model);
void RotateGuizmo(Camera& camera, glm::mat4& model);
void ScaleGuizmo(Camera& camera, glm::mat4& model);
void TransformUI(Camera& camera, glm::mat4& model, MyTransform& transform);