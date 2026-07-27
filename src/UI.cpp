#include "UI.h"
#include "ScreenSpace.h"
#include "savefile.h"

#include <iostream>
#include <fstream>
#include <filesystem>

UI::UI(Shader* backImage,
	HDRI* hdri,
	GBuffer* gbuffer,
	ScreenSpace* ssao,
	ResourceManager* resoManager)
	: m_backImage(backImage),
	m_HDRI(hdri),
	m_GBuffer(gbuffer),
	m_SSAO(ssao),
	m_resoManager(resoManager),
	ImGuiAlpha(0.3f),
	Object(__FUNCTION__) {

	updateAllFiles();
}

utils::utils fpsCounter;

UI::~UI() {}

void displayMatList(int item, static std::int8_t currentItem[], std::vector<const char*> materialFileNames) {
	for (size_t i = 0; i < materialFileNames.size(); i++) {
		bool isSelected = (currentItem[item] == i);
		if (ImGui::Selectable(materialFileNames[i], isSelected))
			currentItem[item] = i;
		if (isSelected)
			ImGui::SetItemDefaultFocus();
	}
}

void createComboBox(const char* comboName, std::vector<const char*>& materialFileNames, static std::int8_t currentItem[], const std::int8_t boxIndex) {
	// ComboBox for Diffuse
	if (ImGui::BeginCombo(comboName, materialFileNames[currentItem[boxIndex]]))
	{
		displayMatList(boxIndex, currentItem, materialFileNames);
		ImGui::EndCombo();
	}
}

ImGuiWindowFlags UI::disableInteraction() {
	if (windowDisabled) { return flagWinDisabled; }
	else { return flagWinEnabled; }
}

void UI::renderMaterialOptions(SettingsMaterial& SetMat, static std::int8_t currentItem[]) {
	// This is probably quite useless, should probably just use std::string instead and not convert these to c_str() everytime
	std::vector<const char*> materialFileNames;
	for (const auto& file : m_materialFileNames) {
		materialFileNames.push_back(file.c_str());
	}

	 // ComboBox for Diffuse
	ImGui::Checkbox("Use Diffuse Texture", &SetMat.useDiffuseTexture);
	if (SetMat.useDiffuseTexture) {
		createComboBox("Diffuse Texture", materialFileNames, currentItem, 0);
	}
	else {
		ImGui::ColorEdit3("Diffuse Color", glm::value_ptr(SetMat.diffuseColor));
	}

	// Add spacing
	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	// ComboBox for Metallic
	ImGui::Checkbox("Use Metallic Texture", &SetMat.useMetallicTexture);
	if (SetMat.useMetallicTexture) {
		createComboBox("Metallic Texture", materialFileNames, currentItem, 1);
	}
	else {
		ImGui::SliderFloat("Metallic Value", &SetMat.metallic, 0.0f, 1.0f);
	}

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	// ComboBox for Roughness
	ImGui::Checkbox("Use Roughness Texture", &SetMat.useRoughnessTexture);
	if (SetMat.useRoughnessTexture) {
		createComboBox("Roughness Texture", materialFileNames, currentItem, 2);
	}
	else {
		ImGui::SliderFloat("Roughness Value", &SetMat.roughness, 0.0f, 1.0f);
	}

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	// ComboBox for Normals
	ImGui::Checkbox("Use Emission Texture", &SetMat.useEmissionTexture);
	if (SetMat.useEmissionTexture) {
		createComboBox("Emission Texture", materialFileNames, currentItem, 3);
	}
	else {
		ImGui::SliderFloat("Emission Value", &SetMat.emission, 0.0f, 10.0f);
	}

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	// ComboBox for Normals
	ImGui::Checkbox("Use Opacity Texture", &SetMat.useOpacityTexture);
	if (SetMat.useOpacityTexture) {
		createComboBox("Opacity Texture", materialFileNames, currentItem, 5); // HOX: was 4, due to MaterialPaths ordering is 5
	}
	else {
		ImGui::SliderFloat("Opacity Value", &SetMat.opacity, 0.0f, 1.0f);
	}

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	// ComboBox for Normal (only texture currently)
	ImGui::Checkbox("Use Normal Texture", &useNormalTexture);
	if (useNormalTexture) {
		createComboBox("Normal Texture", materialFileNames, currentItem, 4); // Was 5
	}

	ImGui::Dummy(ImVec2(0.0, 4.0f));
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
	ImGui::Begin("Control Window", 0, disableInteraction() | ImGuiWindowFlags_MenuBar); // Make a new window

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			static char saveName[128] = ""; // Input field for material name

			if (ImGui::BeginMenu("Open")) {				
				for (auto files : m_saveFiles) {
					if (ImGui::MenuItem(files.c_str())) {
						std::cout << "Opened " + std::string(files) + "\n";

						// Clean up the whole scene
						m_resoManager->cleanResourceManager(m_HDRI);
						m_resoManager->fileLoad(files, m_HDRI);
					}
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Save")) {
				std::cout << "Saved" << std::endl;
				m_resoManager->fileSave(saveName, m_HDRI);	
				updateFiles(m_saveFiles, "Saves/", [this](const std::string& path) { return m_resoManager->FileSystem(path); });
			}
			ImGui::InputText("Write a name for the save file", saveName, IM_ARRAYSIZE(saveName));
		
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	/*if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("Open");
			ImGui::MenuItem("Save");
			ImGui::MenuItem("Save As");
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}*/

	ImGui::Text("Press 'E' to lock/unlock mouse from UI. Feel free to try out different settings!");
	ImGui::Text("Press 'H' to hide the UI window! Toggle 'V' for camera orbit/freecam");
	ImGui::Text("Enable Deferred Rendering for Screen Space effects like SSR and SSAO");
	//ImGui::Text(("Milliseconds Per Frame: " + std::to_string(1000.0 / calculateFPS())).c_str());
	ImGui::Text(("Frames Per Second: " + std::to_string(fpsCounter.calculateFPS())).c_str());

	ImGui::Checkbox("Enable rotation", &meshRotationEnabled);

	shaderBind();

	if (ImGui::Checkbox("Deferred Rendering", &deferredRendering)) {
		if (deferredRendering)
			m_SSAO->constructDeferredRendering();
		else if (!deferredRendering)
			m_SSAO->constructForwardRendering();
	}

	if(deferredRendering) {
		if(ImGui::Button("Set Resolution")) {
			m_SSAO->recreateColorBuffer();
		}
	}

	if (ImGui::Checkbox("Wireframe mode", &wireFrame))
	{
		wireFrame ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	ImGui::Dummy(ImVec2(0.0f, 2.0f));
	ImGui::Text("'Refetch Files' -button updates the available files found in the asset-folder");
	if (ImGui::Button("Refetch Files")) {
		updateAllFiles();
	}
	ImGui::Dummy(ImVec2(0.0f, 2.0f));

	if (ImGui::BeginTabBar("MyTabs"))
	{
		if (ImGui::BeginTabItem("MESH CONTROLS"))
		{
			// Reset options for transformations
			if (ImGui::TreeNode("RESETS"))
			{
				if (ImGui::Button("Reset all the transforms"))
					for (auto models : m_resoManager->getScene()->getModels()) {
						for (auto meshes : models->getMeshes()) {
							meshes->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
							meshes->setScaling(glm::vec3(1.0f, 1.0f, 1.0f));
						}
					}			

				if (ImGui::Button("Reset rotation"))
					for (auto models : m_resoManager->getScene()->getModels()) {
						for (auto meshes : models->getMeshes()) {
							meshes->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
						}
					}

				if (ImGui::Button("Reset scale"))
					for (auto models : m_resoManager->getScene()->getModels()) {
						for (auto meshes : models->getMeshes()) {
							meshes->setScaling(glm::vec3(1.0f, 1.0f, 1.0f));
						}
					}

				ImGui::TreePop();
			}
			ImGui::Separator();

			// Control transformations
			if (ImGui::TreeNode("TRANSFORM/MATERIAL MESHES"))
			{
				ImGui::Text("Below is all the meshes and their transforms");
				std::uint16_t nameIndex = 0;
				for (auto model : m_resoManager->getScene()->getModels()) 
				{
					if (ImGui::TreeNode((model->getModelPath() + ": " + std::to_string(nameIndex)).c_str())) {
						for (size_t i = 0; i < model->getMeshes().size(); i++)
						{
							Mesh* meshes = model->getMeshes()[i];
							if (ImGui::TreeNode(("Mesh " + model->getMeshes()[i]->getDisplayName() + " " + std::to_string(i)).c_str())) // Added an index to the name to avoid duplicates
							{
								glm::vec3 pos = meshes->getPosition();
								if ((ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.01f))) {
									meshes->setPosition(pos); // Update the position if the value changes
								}

								// Control for scale
								ImGui::Checkbox("Scalelock", &scaleLock);
								glm::vec3 scale = meshes->getScaling();
								if (!scaleLock) {
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
									for (size_t i = 0; i < m_resoManager->getScene()->getMaterials().size(); i++)
									{
										bool isSelected = (m_resoManager->getScene()->getMaterials()[i] == currentMat);
										if (ImGui::Selectable(m_resoManager->getScene()->getMaterials()[i]->getName().c_str(), isSelected)) {
											meshes->setMaterial(m_resoManager->getScene()->getMaterials()[i]);  // Set the selected material to the mesh
											m_resoManager->getScene()->updateMeshList();
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
					nameIndex++;
				}
				
				ImGui::TreePop();
			}
			ImGui::Separator();

			// Change mesh
			if (ImGui::TreeNode("REMOVE/ADD MESHES"))
			{
				ImGui::Text("You can load your own 3D-models!");
				ImGui::Text("Supports at least .obj and .fbx");
				ImGui::Text("File path: ../FantaInMySystem/assets/models");

				static int currentItem = 0;
				// Create a combo box with available mesh files
				if (ImGui::BeginCombo("Available models", meshFileNames[currentItem].c_str()))
				{
					for (size_t i = 0; i < meshFileNames.size(); i++) {
						bool isSelected = (currentItem == i);
						if (ImGui::Selectable(meshFileNames[i].c_str(), isSelected)) {
							currentItem = i;
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();  // Ensure selected item is focused
					}

					ImGui::EndCombo();
				}

				// Section for the targeted file searching from a folder
				if (ImGui::Checkbox("Use a Specific Folder Search", &useFolderFiltering))
					m_resoManager->setFolderSearchPath(defaultFolderPath); // Let it be default value

				if (useFolderFiltering) {
					static int currentFolder = 0;
					
					ImGui::Text(("Current folder path: " + m_resoManager->getFolderSearchPath()).c_str());
					if (ImGui::BeginCombo("Available folders", m_folderNames[currentFolder].c_str())) // Should make a method for Combo Boxes
					{
						for (size_t i = 0; i < m_folderNames.size(); i++) {
							bool foundSelected = (currentFolder == i);
							if (ImGui::Selectable(m_folderNames[i].c_str(), foundSelected)) {
								currentFolder = i;
								m_resoManager->setFolderSearchPath(std::string("/textures/") + m_folderNames[currentFolder] + "/"); // Let it be default value
							}
							if (foundSelected)
								ImGui::SetItemDefaultFocus();  // Ensure selected item is focused
						}

						ImGui::EndCombo();
					}

					if (ImGui::Button("Reset folder search path")) {
						m_resoManager->setFolderSearchPath(defaultFolderPath); // Let it be default value
					}
				}

				if (ImGui::Button("Add mesh with automatic textures")) {
					// Load the selected mesh
					std::string selectedItem = ("/models/" + meshFileNames[currentItem]);
					std::vector<Mesh*> newMeshes = m_resoManager->processMeshes(selectedItem, true);
					
					m_resoManager->getScene()->getModels().push_back(new Model(selectedItem, newMeshes));
					m_resoManager->getScene()->updateMeshList();
				}

				// Load selected HDR file and generate the maps for them
				if (ImGui::Button("Add new mesh")) {
					// Load the selected mesh
					std::string selectedItem = ("/models/" + meshFileNames[currentItem]);
					std::vector<Mesh*> newMeshes = m_resoManager->processMeshes(selectedItem);

					m_resoManager->getScene()->getModels().push_back(new Model(selectedItem, newMeshes));

					for (auto& mesh : newMeshes) {
						mesh->setMaterial(m_resoManager->getScene()->getDefaultMaterial()); // Set the default material
					}
					m_resoManager->getScene()->updateMeshList();
				}

				// For selecting and removing meshes
				if (ImGui::TreeNode("Loaded Meshes"))
				{
					for (size_t i = 0; i < m_resoManager->getScene()->getModels().size(); i++) {
						ImGui::Text("Mesh %s", m_resoManager->getScene()->getModels()[i]->getModelPath().c_str());

						if (ImGui::TreeNode(("Child Meshes " + std::to_string(i)).c_str())) { // ("str" + str).c_str(), reminder because I'm a troglodyte
							for (auto meshes : m_resoManager->getScene()->getModels()[i]->getMeshes()) {
								ImGui::Text("Mesh vertex count: %d", meshes->getVertices());
							}
							ImGui::TreePop();
						}

						if (ImGui::Button(("Remove##" + std::to_string(i)).c_str())) { // Prevent duplicated names (duplicated names have uniform actions for all iterations)
							// Remove mesh from vector and cleanup
							delete m_resoManager->getScene()->getModels()[i];
							m_resoManager->getScene()->getModels().erase(m_resoManager->getScene()->getModels().begin() + i);
							m_resoManager->getScene()->updateMeshList();
							break;
						}

						ImGui::Dummy(ImVec2(0.0f, 5.0f));
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
				ImGui::Text("Amount of existing lamps: %zu", m_resoManager->getScene()->getLights().size());

				// Point light addition
				if (ImGui::Button("Add new point light") && m_resoManager->getScene()->getLights().size() < 12) {
					m_resoManager->getScene()->getLights().push_back(FileLights{ glm::vec3(0.0, 0.0, 2.0), glm::vec3(1.0f, 0.5f, 0.31f), 5.0f });
				}
				else if (m_resoManager->getScene()->getLights().size() == 12) {
					ImGui::Text("Maximum amount of lamps reached!!!");
				}

				ImGui::Dummy(ImVec2(5.0f, 5.0f));

				if (ImGui::TreeNode("POINT LAMPS"))
				{
					for (size_t i = 0; i < m_resoManager->getScene()->getLights().size(); i++)
					{
						ImGui::PushID(static_cast<int>(i));	// Each control to be unique
						ImGui::Text("Point Light %zu", i);

						ImGui::DragFloat3("Position", glm::value_ptr(m_resoManager->getScene()->getLights()[i].pos), 0.1f);
						ImGui::ColorEdit3("Color", glm::value_ptr(m_resoManager->getScene()->getLights()[i].color));
						ImGui::InputFloat("Strength", &m_resoManager->getScene()->getLights()[i].strength);

						if (ImGui::Button("Erase point light")) {
							m_resoManager->getScene()->getLights().erase(m_resoManager->getScene()->getLights().begin() + i);
						}
						ImGui::Separator();
						ImGui::PopID();
					}
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("HDRI"))
		{
			if (ImGui::TreeNode("HDRI"))
			{
				static int currentItem = 0;

				ImGui::Text("You can upload your own HDRI files!");
				ImGui::Text("File path: ../FantaInMySystem/assets/HDRI");

				if (ImGui::BeginCombo("Available HDRIs", hdrFileNames[currentItem].c_str()))
				{
					for (size_t i = 0; i < hdrFileNames.size(); i++) {
						bool isSelected = (currentItem == i);
						if (ImGui::Selectable(hdrFileNames[i].c_str(), isSelected))
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
					std::string selectedItem = "/HDRI/" + std::string(hdrFileNames[currentItem]);
					m_HDRI->ProcessHDRI(selectedItem.c_str());
				}

				// Padding
				ImGui::Dummy(ImVec2(0.0f, 7.5f));

				if (deferredRendering) {
					if (ImGui::Checkbox("Lighting Orientation (only for deferred!)", &lightOrientationOn)) {
						if (!m_GBuffer->getLightPass() == 0) {
							shaderSet("worldCoords", lightOrientationOn);
						}
					}
				}				

				if (ImGui::SliderFloat("HDRI Exposure", &HdrExposure, 0.0f, 10.0f)) {
					if (deferredRendering) { // Temporary if else check for now, due to lack of better system of hopping between forward frag, deferred frag and composite
						m_GBuffer->getCompositeShader()->bind();
						m_GBuffer->getCompositeShader()->setUniform("HdrExposure", HdrExposure);
					}
					else {
						m_GBuffer->getCurrentShader()->bind();
						m_GBuffer->getCurrentShader()->setUniform("HdrExposure", HdrExposure);
					}
				}

				if (ImGui::SliderFloat("HDRI Contrast", &HdrContrast, 0.0f, 10.0f)) {
					if (deferredRendering) { // Temporary
						m_GBuffer->getCompositeShader()->bind();
						m_GBuffer->getCompositeShader()->setUniform("HdrContrast", HdrContrast);
					}
					else {
						m_GBuffer->getCurrentShader()->bind();
						m_GBuffer->getCurrentShader()->setUniform("HdrContrast", HdrContrast);
					}
					
				}

				if (ImGui::DragFloat3("HueChanges", glm::value_ptr(HueChanges), 0.01f)) {
					shaderSet("HueChanges", HueChanges);
				}

				// Load selected HDR file and generate the maps for them
				if (ImGui::Button("Reset Exposure/Contrast")) {
					HdrContrast = 2.2f;
					HdrExposure = 1.0f;
					HueChanges = glm::vec3(1.0f);

					shaderSet("HdrContrast", HdrContrast);
					shaderSet("HdrExposure", HdrExposure);
					shaderSet("HueChanges", HueChanges);
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
					ImGui::Text("Drop textures into: ../FantaInMySystem/assets/backgrounds");
					std::vector<std::string> backgroundFiles = m_resoManager->FileSystem((std::string(ASSET_DIR) + "/backgrounds/"));
					
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
					if (ImGui::Button("Reset Background Exposure/Contrast")) {
						backContrast = 2.2f;
						backExposure = 1.0f;
						m_backImage->bind();
						m_backImage->setUniform("backExposure", backExposure);
						m_backImage->setUniform("backContrast", backContrast);
					}

					if (ImGui::Button("Set Background Texture")) {
						// Clean up background texture
						m_HDRI->cleanBackgroundTexture();
						std::string selectedItem = "/backgrounds/" + backgroundFiles[currentBackground];
						Texture* newBackground = m_resoManager->loadTexture(selectedItem.c_str(), true);
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
		if (deferredRendering) {
			if (ImGui::BeginTabItem("SCREEN-SPACE"))
			{		
				if (ImGui::TreeNode("SSAO"))
				{			
					SSAO_Settings& ssao = m_SSAO->getSSAO_Settings();
					if (ImGui::Checkbox("Use SSAO", &ssao.useSSAO)) { // Could be better
						m_SSAO->recreateColorBuffer();
						ssao.dirty = true;
						m_SSAO->updateSSAOUniforms();	
					}				
					if (ImGui::InputInt("Kernel Samples", &ssao.kernelSize)) 
						ssao.dirty = true;
					if (ImGui::InputFloat("Radius", &ssao.radius)) 
						ssao.dirty = true;
					if (ImGui::InputFloat("Bias", &ssao.bias)) 
						ssao.dirty = true;
					if (ImGui::InputFloat("Occlusion Strength", &ssao.occlusionStrength)) 
						ssao.dirty = true;	
					if (ImGui::Checkbox("Clamped Midtones", &ssao.clampedMidTones)) 
						ssao.dirty = true;
					
					ImGui::TreePop();
				}
				ImGui::Separator();

				if (ImGui::TreeNode("SSR"))
				{
					SSR_Settings& ssr = m_SSAO->getSSR_Settings();
					if (ImGui::Checkbox("Use SSR", &ssr.useSSR)) 
						m_SSAO->recreateColorBuffer();
					if (ImGui::Checkbox("Use Temporary Accumulation - TA", &ssr.useTA))
						ssr.dirty = true;
					if (ImGui::Checkbox("Use Roughness Ray Scattering (recommended with TA)", &ssr.useRayScattering))
						ssr.dirty = true;			
					if (ImGui::InputInt("maxSteps", &ssr.maxSteps)) 
						ssr.dirty = true;			
					if (ImGui::InputFloat("thickness", &ssr.thickness))
						ssr.dirty = true;
					if (ImGui::InputFloat("rayDirMin", &ssr.rayDirMin))
						ssr.dirty = true;
					if (ImGui::Checkbox("Use Binary Refinement", &ssr.useBinaryRefinement))
						ssr.dirty = true;

					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}
		}
		if (ImGui::BeginTabItem("MATERIALS"))
		{
			if (ImGui::TreeNode("ADD MATERIAL"))
			{
				static std::int8_t currentItem[6] = { 0, 0, 0, 0, 0, 0 }; // One for each texture type
				static char materialName[128] = ""; // Input field for material name
				SettingsMaterial& SetMat = m_settingsCreateMat;

				renderMaterialOptions(SetMat, currentItem);

				ImGui::InputText("Set name for the material", materialName, IM_ARRAYSIZE(materialName));
				if (ImGui::Button("Create a new material")) {
					stbi_set_flip_vertically_on_load(false);

					// In case, where the texture is not used ("" is handled as no texture)
					std::string usePaths[6];
					bool useTextures[5] = { SetMat.useDiffuseTexture, SetMat.useMetallicTexture, SetMat.useRoughnessTexture, SetMat.useEmissionTexture, SetMat.useOpacityTexture};
					for (std::uint8_t i = 0; i < 5; i++) {
						usePaths[i] = useTextures[i] ? "/textures/" + m_materialFileNames[currentItem[0]] : "";
					}

					usePaths[5] = "EmptyNormal.png"; // Set the default normal map name
					if (useNormalTexture)
						usePaths[5] = m_materialFileNames[currentItem[5]];

					// Set the default material
					m_resoManager->createMaterial(m_resoManager->createMaterialPaths(materialName, usePaths, SetMat));
					m_resoManager->getScene()->updateMeshList();

					for (const auto& texture : m_resoManager->getTrackedTextures())
						std::cout << "Current textures: " << texture.first << std::endl;

				}

				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("EDIT MATERIALS"))
			{
				static std::int8_t select = 0;
				static std::int8_t comboBoxSelection[6] = { 0, 0, 0, 0, 0, 0 }; // One for each texture type
				const std::int8_t comboBoxSize = sizeof(comboBoxSelection) / sizeof(comboBoxSelection[0]);

				std::vector<Material*>& materials = m_resoManager->getScene()->getMaterials();
				SettingsMaterial& SetMat = m_settingsEditMat;

				if (!materials.empty()) {
					const char* materialName = materials[select] ? materials[select]->getName().c_str() : "None"; // transform string into c_str() or set name to "None"

					if (ImGui::BeginCombo("Material", materialName))
					{
						for (size_t i = 0; i < materials.size(); i++) {
							bool isSelected = (materials[i] == materials[select]);
							if (ImGui::Selectable(m_resoManager->getScene()->getMaterials()[i]->getName().c_str(), isSelected)) {
								select = i; // Set the selected index
								for (size_t j = 0; j < comboBoxSize; j++) { // comboBoxSelection size
									Texture* foundTex = m_resoManager->findTexture(materials[select]->getTextures()[j]);
									if (foundTex == nullptr) // solid values are nullptrs
										continue;
									
									std::string texFilename = foundTex->getTextureFilename();
									for (size_t k = 0; k < m_materialFileNames.size(); k++) {
										//std::cout << "Compared: " << m_materialFileNames[k] << " - Target:" << texFilename << std::endl;
										if (m_materialFileNames[k] == texFilename) { // maybe store the material names into an unordered_map
											comboBoxSelection[j] = k;
											break;
										}
									}
								}

								// Set material properties and bools
								m_resoManager->setMaterialParams(SetMat, materials[select]);
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();  // Ensure selected item is focused
						}
						ImGui::EndCombo();
					}

					renderMaterialOptions(SetMat, comboBoxSelection);

					if (ImGui::Button("Apply edited material"))
					{
						stbi_set_flip_vertically_on_load(false);

						// In case, where the texture is not used ("" is handled as no texture)
						std::string usePaths[6];
						bool useTextures[5] = { SetMat.useDiffuseTexture, SetMat.useMetallicTexture, SetMat.useRoughnessTexture, SetMat.useEmissionTexture, SetMat.useOpacityTexture };
						for (std::uint8_t i = 0; i < 5; i++) {
							usePaths[i] = useTextures[i] ? "/textures/" + m_materialFileNames[comboBoxSelection[0]] : "";
						}
						usePaths[5] = "EmptyNormal.png";
						if (useNormalTexture)
							usePaths[5] = m_materialFileNames[comboBoxSelection[5]];

						MaterialPaths newMaterialParams = m_resoManager->createMaterialPaths(std::string(materials[select]->getName()), usePaths, SetMat);

						// Material to be edited and the parameters
						m_resoManager->editMaterial(materials[select], newMaterialParams);
						m_resoManager->getScene()->updateMeshList();
					}

					ImGui::Dummy(ImVec2(0.0f, 4.0f));

					ImGui::Text("This removes the selected material");
					if (ImGui::Button("Remove selected material")) {
						m_resoManager->replaceMaterials(materials[select], m_resoManager->getScene()->getDefaultMaterial());
						delete materials[select];
						materials.erase(materials.begin() + select);	
						select = 0;
						m_resoManager->clearUnusedTextures();
						m_resoManager->getScene()->updateMeshList();
					}
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