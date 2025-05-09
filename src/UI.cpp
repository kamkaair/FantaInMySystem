#include "UI.h"
#include "ssao.h"

UI::UI(Shader* backImage,
	TextureLoading* texLoad,
	HDRI* hdri,
	GBuffer* gbuffer,
	SSAO* ssao)
	: m_backImage(backImage),
	m_texLoading(texLoad),
	m_HDRI(hdri),
	m_GBuffer(gbuffer),
	m_SSAO(ssao),
	ImGuiAlpha(0.3f), 
	Object(__FUNCTION__) {}

utils::utils fpsCounter;

UI::~UI() {
	// Clean up all the meshes
	for (size_t i = 0; i < m_meshes.size(); i++) {
		delete m_meshes[i];
	}
	m_meshes.clear();
}

void displayMatList(int item, static int currentItem[], std::vector<const char*> materialFileNames) {
	for (size_t i = 0; i < materialFileNames.size(); i++) {
		bool isSelected = (currentItem[item] == i);
		if (ImGui::Selectable(materialFileNames[i], isSelected))
			currentItem[item] = i;
		if (isSelected)
			ImGui::SetItemDefaultFocus();
	}
}

ImGuiWindowFlags UI::disableInteraction() {
	if (windowDisabled) { return flagWinDisabled; }
	else { return flagWinEnabled; }
}

void UI::ImGuiStyleSetup()
{
	// Using the "Dracula Style" made by Trippasch in ImGui GitHub forum section (I've made few alterations to this color scheme)
	// https://github.com/ocornut/imgui/issues/707#issuecomment-1372640066

	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
	colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Border
	colors[ImGuiCol_Border] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
	colors[ImGuiCol_BorderShadow] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.24f };

	// Text
	colors[ImGuiCol_Text] = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
	colors[ImGuiCol_TextDisabled] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

	// Headers
	colors[ImGuiCol_Header] = ImVec4{ 0.13f, 0.13f, 0.17, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Buttons
	colors[ImGuiCol_Button] = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };

	// Popups
	colors[ImGuiCol_PopupBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 0.92f };

	// Slider
	colors[ImGuiCol_SliderGrab] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.54f };
	colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.54f };

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.13f, 0.13, 0.17, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4{ 0.48f, 0.72f, 0.89f, 0.49f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.50f, 0.69f, 0.99f, 0.68f };
	colors[ImGuiCol_TabActive] = ImVec4{ 0.80f, 0.50f, 0.50f, 1.00f };
	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
	colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };

	// Seperator
	colors[ImGuiCol_Separator] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };
	colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };
	colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 1.0f };

	// Resize Grip
	colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
	colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.29f };
	colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 0.29f };

	auto& style = ImGui::GetStyle();
	style.TabRounding = 4;
	style.ScrollbarRounding = 9;
	style.WindowRounding = 7;
	style.GrabRounding = 3;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ChildRounding = 4;
}

void UI::ImGuiDraw()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGuiAlpha);
	ImGuiStyleSetup();
	ImGui::Begin("Control Window", 0, disableInteraction()); // Make a new window

	ImGui::Text("Sup broidi, press 'E' to lock/unlock mouse. Feel free to try out different settings!");
	ImGui::Text("Press 'H' to hide the window! Toggle 'V' for camera orbit/freecam");
	//ImGui::Text(("Milliseconds Per Frame: " + std::to_string(1000.0 / calculateFPS())).c_str());
	ImGui::Text(("Frames Per Second: " + std::to_string(fpsCounter.calculateFPS())).c_str());

	ImGui::Checkbox("Enable rotation", &meshRotationEnabled);

	shaderBind();

	if (ImGui::Checkbox("Deferred rendering", &deferredRendering)) {
		if (deferredRendering) {
			glUseProgram(0); // Unbind any active shader
			m_GBuffer->constructDeferredShaders();
			m_GBuffer->deconstructForwardShaders();
			m_SSAO->constructSSAO();
		}
		else if (!deferredRendering) {
			glUseProgram(0); // Unbind any active shader
			m_SSAO->deconstructSSAO();
			m_GBuffer->constructForwardShaders();
			m_GBuffer->deconstructDeferredShaders();
		}
	}

	if(deferredRendering) {
		glDisable(GL_BLEND);
		if(ImGui::Button("Set Resolution")) {
			m_GBuffer->CleanUpGBuffer();
			m_GBuffer->setResolution(m_GBuffer->getWidth(), m_GBuffer->getHeight());
			m_GBuffer->constructGBuffer();
			m_SSAO->recreateColorBuffer();
		}
	}
	else {
		glEnable(GL_BLEND);
	}

	if (ImGui::Checkbox("Wireframe mode", &wireFrame))
	{
		wireFrame ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//ImGui::Separator();

	if (ImGui::BeginTabBar("MyTabs"))
	{
		if (ImGui::BeginTabItem("MESH CONTROLS"))
		{
			// Reset options for transformations
			if (ImGui::TreeNode("RESETS"))
			{

				if (ImGui::Button("Reset all the transforms"))
					for (auto meshes : m_meshes) {
						meshes->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
						meshes->setScaling(glm::vec3(1.0f, 1.0f, 1.0f));
					}

				if (ImGui::Button("Reset rotation"))
					for (auto meshes : m_meshes) {
						meshes->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
					}

				if (ImGui::Button("Reset scale"))
					for (auto meshes : m_meshes) {
						meshes->setScaling(glm::vec3(1.0f, 1.0f, 1.0f));
					}

				ImGui::TreePop();
			}
			ImGui::Separator();

			// Control transformations
			if (ImGui::TreeNode("TRANSFORM/MATERIAL MESHES"))
			{
				ImGui::Text("Below is all the meshes and their transforms");
				for (size_t i = 0; i < m_meshes.size(); i++)
				{
					Mesh* meshes = m_meshes[i];

					//std::to_string(i)).c_str())
					//			m_meshes.back()->setMaterial(m_materials[0]);
					if (ImGui::TreeNode(("Mesh " + m_meshes[i]->getDisplayName()).c_str()))
					{
						//if (ImGui::Button("Hide mesh", ImVec2(128, 32))) {
						//	meshHide = !meshHide;
						//	if (meshHide) {
						//		//m_hiddenMeshes.push_back(m_meshes[i]);
						//		m_meshes.erase(m_meshes.begin() + i);
						//		printf("erase mesh");
						//		//meshHide = false;
						//	}

						//	else {
						//		m_meshes.push_back(m_meshes[i]);
						//		printf("add mesh");
						//		//m_hiddenMeshes.erase(m_hiddenMeshes.begin() + i);
						//		//meshHide = true;
						//	}
						//	
						//}

						//std::string koko = "Object Name: " + m_meshes[i]->getBackgroundName();
						//ImGui::Text(koko.c_str());
						ImGui::Text(m_meshes[i]->getBackgroundName().c_str());
						ImGui::Dummy(ImVec2(0.0f, 7.5f));

						glm::vec3 pos = meshes->getPosition();
						if ((ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.01f))) {
							meshes->setPosition(pos); // Update the position if the value changes
						}

						// Control for scale
						ImGui::Checkbox("Scalelock", &scaleLock);
						glm::vec3 scale = meshes->getScaling();
						if (!scaleLock) {
							// Reset the uniform scale and original scale when ScaleLock is disabled
							//originalScale = scale;
							//totalScale = 1.0f;

							// Set indiviudal XYZ scaling
							if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f)) {
								meshes->setScaling(scale);
							}
						}
						else
						{
							// Set scaling uniformally
							ImGui::Text(glm::to_string(scale).c_str());
							if (ImGui::DragFloat("Scale", &totalScale, 0.01f))
							{
								float scaleSet = totalScale;
								scale = originalScale * scaleSet;
								meshes->setScaling(scale);
							}
							//if (ImGui::IsItemDeactivated()) {
							//	//originalScale = scale;
							//	//totalScale = 1.0f;
							//}
						}

						// Control for rotation
						glm::vec3 rotation = meshes->getRotation();
						if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.01f))
						{
							meshes->setRotation(rotation);
						}

						// Control for material
						Material* currentMat = meshes->getMaterial();
						const char* changeMat = currentMat ? currentMat->getName().c_str() : "None"; // get a const char

						if (ImGui::BeginCombo("Material", changeMat))  // Combo box to choose material
						{
							for (size_t i = 0; i < m_texLoading->getMaterials().size(); i++)
							{
								bool isSelected = (m_texLoading->getMaterials()[i] == currentMat);
								if (ImGui::Selectable(m_texLoading->getMaterials()[i]->getName().c_str(), isSelected))
								{
									meshes->setMaterial(m_texLoading->getMaterials()[i]);  // Set the selected material to the mesh
								}
								if (isSelected)
									ImGui::SetItemDefaultFocus();  // Ensure selected item is focused
							}
							ImGui::EndCombo();
						}
						ImGui::TreePop();
					}

				}
				ImGui::TreePop();
			}
			ImGui::Separator();

			// Change mesh
			if (ImGui::TreeNode("REMOVE/ADD MESHES"))
			{
				ImGui::Text("You can load your own 3D-models!");
				ImGui::Text("Supports at least .obj and .fbx");
				ImGui::Text("File path: ../opengl-graphicsengine/assets/models");
				std::vector<std::string> meshFiles = m_texLoading->FileSystem((std::string(ASSET_DIR) + "/models/"));

				std::vector<const char*> meshFileNames;
				for (const auto& file : meshFiles)
				{
					// file into c_str()
					meshFileNames.push_back(file.c_str());
				}

				static int currentItem = 0;

				// Create a combo box with available mesh files
				if (ImGui::BeginCombo("Available models", meshFileNames[currentItem]))
				{
					for (size_t i = 0; i < meshFileNames.size(); i++) {
						bool isSelected = (currentItem == i);
						if (ImGui::Selectable(meshFileNames[i], isSelected))
						{
							//meshFileNames[i];
							currentItem = i;
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();  // Ensure selected item is focused
					}

					ImGui::EndCombo();
				}
				// Load selected HDR file and generate the maps for them
				if (ImGui::Button("Add new mesh"))
				{
					// Load the selected mesh
					std::string selectedItem = (std::string(ASSET_DIR) + "/models/" + meshFiles[currentItem]);
					auto newMeshes = m_texLoading->loadMeshes(selectedItem, m_texLoading->getMaterials(), meshFiles[currentItem]);

					// Add the new mesh to the std::vector
					for (auto& mesh : newMeshes)
					{
						m_meshes.push_back(mesh);
						m_meshes.back()->setMaterial(m_texLoading->getMaterials()[0]);
					}
				}

				// For selecting and removing meshes
				if (ImGui::TreeNode("Loaded Meshes"))
				{
					for (size_t i = 0; i < m_meshes.size(); i++)
					{
						ImGui::Text("Mesh %s", m_meshes[i]->getDisplayName().c_str());
						ImGui::Text("Vertex count: %d", m_texLoading->getVertices()[i]);
						//ImGui::Text("Vertex count: " + std::to_string(m_texLoading->getVertices()[i]).c_str());

						// Show Remove button next to each mesh
						if (ImGui::Button(("Remove##" + std::to_string(i)).c_str()))
						{
							// Remove mesh from vector and cleanup
							delete m_meshes[i];  // Clean up memory if necessary
							m_meshes.erase(m_meshes.begin() + i);
							
							//delete m_texLoading->getVertices()[i];
							//m_texLoading->getVertices().erase(m_texLoading->getVertices().begin() + i); // Erase the vertex amount

							break;  // Exit loop since vector has changed
						}
					}
					ImGui::TreePop();
				}

				ImGui::TreePop();
			}
			ImGui::Separator();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("LAMP"))
		{
			if (ImGui::TreeNode("LAMPS"))
			{
				ImGui::Text("Amount of existing lamps: %zu", pointLightPos.size());
				if (ImGui::TreeNode("Positions"))
				{
					// We need pointLight array and make an individual DragFloat3 for all of them
					for (size_t i = 0; i < pointLightPos.size(); i++)
					{
						ImGui::PushID(static_cast<int>(i));	// Each control to be unique
						ImGui::Text("Point Light %zu", i);
						if (ImGui::DragFloat3("Position", glm::value_ptr(pointLightPos[i]), 0.1f));  // Directly modify pointLightPos
						if (ImGui::Button("Erase point light")) {
							pointLightPos.erase(pointLightPos.begin() + i);
							pointLightColor.erase(pointLightColor.begin() + i);
						}
						ImGui::PopID();

					}
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Colors"))
				{
					for (size_t i = 0; i < pointLightColor.size(); i++)
					{
						ImGui::PushID(static_cast<int>(i));	// Each control to be unique
						ImGui::Text("Point Light %zu", i);
						//ImGui::DragFloat3("Color", glm::value_ptr(pointLightColor[i]), 0.1f);  // Directly modify pointLightColor
						// Edit a color stored as 4 floats
						ImGui::ColorEdit3("Color", glm::value_ptr(pointLightColor[i]));
						ImGui::PopID();
					}
					ImGui::TreePop();
				}

				if (ImGui::Button("Add new point light") && pointLightPos.size() < 12)
				{
					pointLightPos.push_back(glm::vec3(0.0, 0.0, 2.0));
					pointLightColor.push_back(glm::vec3(1.0f, 0.5f, 0.31f));
				}
				else if (pointLightPos.size() == 12)
				{
					ImGui::Text("Maximum amount of lamps reached!!!");
				}

				//if (ImGui::Button("Erase the top of the list point light") && pointLightPos.size() > 0)
				//{
				//	pointLightPos.erase(pointLightPos.begin());
				//	pointLightColor.erase(pointLightColor.begin());
				//}

				// COMMENTED OUT FOR NOW
				if (ImGui::SliderFloat("Lamp Strength", &lampStrength, 0.0f, 100.0f))  // Directly modify pointLightColor
				{
					//shaderBind();
					shaderSet("LampStrength", lampStrength);
				}
				ImGui::TreePop();
			}
			ImGui::Separator();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("HDRI"))
		{
			if (ImGui::TreeNode("HDRI"))
			{
				std::vector<std::string> hdrFiles = m_texLoading->FileSystem((std::string(ASSET_DIR) + "/HDRI/"));

				std::vector<const char*> hdrFileNames;
				for (const auto& file : hdrFiles)
				{
					// file into c_str()
					hdrFileNames.push_back(file.c_str());
				}

				static int currentItem = 0;

				ImGui::Text("You can upload your own HDRI files!");
				ImGui::Text("File path: ../opengl-graphicsengine/assets/HDRI");
				//const char* allText = hdrFiles;
				if (ImGui::BeginCombo("Available HDRIs", hdrFileNames[currentItem]))
				{
					for (size_t i = 0; i < hdrFileNames.size(); i++) {
						bool isSelected = (currentItem == i);
						if (ImGui::Selectable(hdrFileNames[i], isSelected))
						{
							hdrFileNames[i];
							currentItem = i;
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();  // Ensure selected item is focused
					}

					ImGui::EndCombo();
				}
				// Load selected HDR file and generate the maps for them
				if (ImGui::Button("Set HDRI")) {
					m_HDRI->cleanUpHDRI();
					std::string selectedItem = "/HDRI/" + hdrFiles[currentItem];
					m_HDRI->ProcessHDRI(selectedItem.c_str());
				}

				// Padding
				ImGui::Dummy(ImVec2(0.0f, 7.5f));

				if (ImGui::Checkbox("Lighting Orientation (only for deferred!)", &lightOrientationOn)) {
					if (!m_GBuffer->getLightPass() == 0) {
						shaderSet("worldCoords", lightOrientationOn);
					}
				}

				if (ImGui::SliderFloat("HDRI Exposure", &HdrExposure, 0.0f, 10.0f)) {
					shaderSet("HdrExposure", HdrExposure);
				}

				if (ImGui::SliderFloat("HDRI Contrast", &HdrContrast, 0.0f, 10.0f)) {
					shaderSet("HdrContrast", HdrContrast);
				}

				if (ImGui::SliderFloat("Hue", &HueChange, -10.0f, 10.0f)) {
					shaderSet("HueChange", HueChange);
				}

				// Load selected HDR file and generate the maps for them
				if (ImGui::Button("Reset Exposure/Contrast")) {
					HdrContrast = 2.2f;
					HdrExposure = 1.0f;
					HueChange = 0.0f;

					shaderSet("HdrContrast", HdrContrast);
					shaderSet("HdrExposure", HdrExposure);
					shaderSet("HueChange", HueChange);
				}

				// Padding
				ImGui::Dummy(ImVec2(0.0f, 7.5f));
				ImGui::Separator();
				ImGui::Dummy(ImVec2(0.0f, 7.5f));

				// ----------------------------------------
				// BACKGROUND RENDERING
				//----------------------------------------

				ImGui::Text("Background color / texture");
				ImGui::Dummy(ImVec2(0.0f, 2.5f));

				// Select the background
				ImGui::Combo("Background selection", &backgroundMode, backgroundOptions, IM_ARRAYSIZE(backgroundOptions));
				ImGui::Dummy(ImVec2(0.0f, 5.0f));

				// For textures based background
				if (backgroundMode == 1) {
					ImGui::Text("Attention! .jpg images might cause issues, use .pngs!");
					ImGui::Text("Drop textures into: ../opengl-graphicsengine/assets/backgrounds");
					std::vector<std::string> backgroundFiles = m_texLoading->FileSystem((std::string(ASSET_DIR) + "/backgrounds/"));
					
					std::vector<const char*> backgroundFileNames;
					for (const auto& file : backgroundFiles)
					{
						// file into c_str()
						backgroundFileNames.push_back(file.c_str());
					}

					static int currentBackground = 0;

					 // Make a combobox for all the background textures
					if (ImGui::BeginCombo("Background Texture", backgroundFileNames[currentBackground]))
					{
						for (size_t i = 0; i < backgroundFileNames.size(); i++) {
							bool isSelected = (currentBackground == i);
							if (ImGui::Selectable(backgroundFileNames[i], isSelected))
								currentBackground = i;
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					if (ImGui::SliderFloat("Background Texture Exposure", &backExposure, -10.0f, 10.0f)) {
						m_backImage->bind();
						m_backImage->setUniform("backExposure", backExposure);
					}

					if (ImGui::SliderFloat("Background Texture Contrast", &backContrast, -10.0f, 10.0f)) {
						m_backImage->bind();
						m_backImage->setUniform("backContrast", backContrast);
					}

					// Load selected HDR file and generate the maps for them
					if (ImGui::Button("Reset Exposure/Contrast")) {
						backContrast = 2.2f;
						m_backImage->setUniform("HdrContrast", HdrContrast);
						backExposure = 1.0f;
						m_backImage->setUniform("HdrExposure", HdrExposure);
					}

					if (ImGui::Button("Set Background Texture")) {
						// Clean up background texture
						m_HDRI->cleanBackgroundTexture();
						std::string selectedItem = "/backgrounds/" + backgroundFiles[currentBackground];
						Texture* newBackground = m_texLoading->loadTexture(std::string(ASSET_DIR) + selectedItem.c_str()); // Crash happens here!!! Error loading texture
						m_HDRI->setBackgroundTexture(newBackground);
					}
				}
				// For solid colored background!
				else if (backgroundMode == 2) {
					if (ImGui::ColorEdit4("Background color", backgroundColor)) {
						glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
					}
				}

				ImGui::TreePop();
			}
			ImGui::Separator();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("POST-PROCESS"))
		{
			if (ImGui::TreeNode("SSAO"))
			{
				if (ImGui::InputInt("Kernel Samples", &kernelSize)) {
					m_SSAO->getSsaoShader()->bind();
					m_SSAO->getSsaoShader()->setUniform("kernelSize", kernelSize);
				}
				if (ImGui::InputFloat("Radius", &radius)) {
					m_SSAO->getSsaoShader()->bind();
					m_SSAO->getSsaoShader()->setUniform("radius", radius);
				}
				if (ImGui::InputFloat("Bias", &bias)) {
					m_SSAO->getSsaoShader()->bind();
					m_SSAO->getSsaoShader()->setUniform("bias", bias);
				}
				if (ImGui::InputFloat("Occlusion Strength", &aoStrength)) {
					if (!m_GBuffer->getLightPass() == 0) {
						//shaderBind();
						shaderSet("aoStrength", aoStrength);
					}
				}
				if (ImGui::Checkbox("Clamped Midtones", &aoMidTones)) {
					if (!m_GBuffer->getLightPass() == 0) {
						//shaderBind();
						shaderSet("aoTone", aoMidTones);
					}
				}

				ImGui::TreePop();
			}
			ImGui::Separator();

			ImGui::EndTabItem();
		}
	
		if (ImGui::BeginTabItem("MATERIALS"))
		{
			if (ImGui::TreeNode("ADD MATERIAL"))
			{
				// Use the member variable
				SettingsMaterial& SetMat = m_settingsMaterial;

				std::vector<std::string> materialFiles = m_texLoading->FileSystem((std::string(ASSET_DIR) + "/textures/"));

				std::vector<const char*> materialFileNames;
				for (const auto& file : materialFiles)
				{
					// file into c_str()
					materialFileNames.push_back(file.c_str());
				}

				static int currentItem[4] = { 0, 0, 0, 0 }; // One for each texture type
				static char materialName[128] = ""; // Input field for material name

				 // ComboBox for Diffuse
				ImGui::Checkbox("Use Diffuse Texture", &SetMat.useDiffuseTexture);
				if (SetMat.useDiffuseTexture) {
					if (ImGui::BeginCombo("Diffuse Texture", materialFileNames[currentItem[0]]))
					{
						displayMatList(0, currentItem, materialFileNames);
						ImGui::EndCombo();
					}
				}
				else {
					ImGui::ColorEdit3("Diffuse Color", glm::value_ptr(SetMat.diffuseColor));
				}
				
				// Add spacing
				ImGui::Dummy(ImVec2(0.0f, 5.0f));

				// ComboBox for Metallic
				ImGui::Checkbox("Use Metallic Texture", &SetMat.useMetallicTexture);
				if (SetMat.useMetallicTexture) {
					if (ImGui::BeginCombo("Metallic Texture", materialFileNames[currentItem[1]]))
					{
						displayMatList(1, currentItem, materialFileNames);
						ImGui::EndCombo();
					}
				}
				else {
					ImGui::SliderFloat("Metallic Value", &SetMat.metallic, 0.0f, 1.0f);
				}

				ImGui::Dummy(ImVec2(0.0f, 5.0f));

				// ComboBox for Roughness
				ImGui::Checkbox("Use Roughness Texture", &SetMat.useRoughnessTexture);
				if (SetMat.useRoughnessTexture) {
					if (ImGui::BeginCombo("Roughness Texture", materialFileNames[currentItem[2]]))
					{
						displayMatList(2, currentItem, materialFileNames);
						ImGui::EndCombo();
					}
				}
				else {
					ImGui::SliderFloat("Roughness Value", &SetMat.roughness, 0.0f, 1.0f);
				}

				ImGui::Dummy(ImVec2(0.0f, 5.0f));

				// ComboBox for Normal (only texture currently)
				ImGui::Text("Normal Map");
				if (ImGui::BeginCombo("Normal Texture", materialFileNames[currentItem[3]]))
				{
					displayMatList(3, currentItem, materialFileNames);
					ImGui::EndCombo();
				}

				ImGui::InputText("Set name for the material", materialName, IM_ARRAYSIZE(materialName));
				if (ImGui::Button("Create a new material"))
				{
					stbi_set_flip_vertically_on_load(false);

					Material* newMaterial = m_texLoading->getMaterialMap()[m_texLoading->getMaterialMap().size() + 1] = m_texLoading->checkAndAddMaterial(m_texLoading->loadTextureSet(
						ASSET_DIR + std::string("/textures/" + materialFiles[currentItem[0]]), // Diffuse
						ASSET_DIR + std::string("/textures/" + materialFiles[currentItem[1]]), // Metallic
						ASSET_DIR + std::string("/textures/" + materialFiles[currentItem[2]]), // Roughness
						ASSET_DIR + std::string("/textures/" + materialFiles[currentItem[3]]) // Normal
					), materialName);

					// Set material properties for the struct
					newMaterial->diffuseColor = SetMat.diffuseColor;
					newMaterial->roughness = SetMat.roughness;
					newMaterial->metallic = SetMat.metallic;

					newMaterial->useDiffuseTexture = SetMat.useDiffuseTexture;
					newMaterial->useMetallicTexture = SetMat.useMetallicTexture;
					newMaterial->useRoughnessTexture = SetMat.useRoughnessTexture;

					m_texLoading->getMaterialMap()[m_texLoading->getMaterialMap().size() + 1] = newMaterial;
				}

				ImGui::TreePop();
			}
			ImGui::Separator();

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}