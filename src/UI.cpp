#include "UI.h"
#include "ssao.h"
#include "savefile.h"

#include <iostream>
#include <fstream>
#include <filesystem>

UI::UI(Shader* backImage,
	TextureLoading* texLoad,
	HDRI* hdri,
	GBuffer* gbuffer,
	SSAO* ssao,
	Scene* scene)
	: m_backImage(backImage),
	m_texLoading(texLoad),
	m_HDRI(hdri),
	m_GBuffer(gbuffer),
	m_SSAO(ssao),
	m_scene(scene),
	ImGuiAlpha(0.3f), 
	Object(__FUNCTION__) {
	updateMeshFiles();
}

utils::utils fpsCounter;

UI::~UI() {}

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

void checkDuplicateMaterial(bool seen) {

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
				std::vector<std::string> hdrFiles = m_texLoading->FileSystem((std::string(ASSET_DIR) + "/Saves/"));
				for (auto files : hdrFiles) {
					if (ImGui::MenuItem(files.c_str())) {
						std::cout << "Opened " + files + "\n";

						// Clean up the whole scene
						m_scene->cleanupScene();
						m_texLoading->cleanupTextures();
						m_HDRI->cleanUpHDRI();
						m_HDRI->cleanBackgroundTexture();

						// Deserialize the object
						SaveFile restored = SaveFile::deserialize(std::string(ASSET_DIR) + "/Saves/" + files);

						std::vector<Material*> materials = m_texLoading->MaterialsPushback(restored.getPathNames());
						m_scene->getMaterials() = materials;

						std::vector<Model*> models;
						m_texLoading->loadMeshes(models, restored.getFileMeshes()); // Preset modes from 0 - 3
						m_scene->getModels() = models;

						std::cout << "Background tex load: " << restored.getBackgroundTexPath() << std::endl;

						Texture* backgroundImage = m_texLoading->loadTexture(restored.getBackgroundTexPath());
						m_HDRI->setBackgroundTexture(backgroundImage);

						// Load the HDR texture and create all the HDRI maps
						m_HDRI->ProcessHDRI(restored.getHdriPath().c_str());

						// Set up lights and color
						m_scene->getLights() = restored.getLightData();

						m_scene->constructScene(models, materials, restored.getLightData());

						// Just in case, if no materials were added
						if (m_scene->getMaterials().empty()) {
							m_texLoading->checkAndAddMaterial(m_texLoading->loadTextureSet(
								std::string("/textures/checkerboard.png"),
								std::string("/textures/checkerboard.png"),
								std::string("/textures/checkerboard.png"),
								std::string("/textures/checkerboardNormal.png")
							), "Default Material");
							std::cout << "Material empty, creating Default Material" << std::endl;
						}
					}
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();

		    /*bool show_dialog = true;
			ImGui::Begin("Dialogx1", &show_dialog, ImGuiWindowFlags_NoCollapse);
			ImGui::End();*/
			if (ImGui::MenuItem("Save")) {
				std::cout << "Saved" << std::endl;
				// Light positions, colors and light strength

				// And all the meshes and materials
				std::vector<MaterialPaths> materialPath;
				std::vector<Material*> checkMaterials;
				std::vector<FileModels> fileModels;
				int texIndex = 0;
				for (auto model : m_scene->getModels()) {
					std::vector<FileMeshes> fileMeshes;
					for(auto mesh : model->getMeshes()) {
						bool seen = false;
						for (auto earlierMat : checkMaterials) { // Silly dinky way of detecting, whether the material is already in use
							for (int i = 0; i < mesh->getMaterial()->getTextures().size(); i++) {
								if (mesh->getMaterial()->getTextures()[i] == earlierMat->getTextures()[i]) {
									std::cout << "Detected earlier material!!" << std::endl;
									seen = true;
									break;
								}
							}
						}

						if (!seen) {
							std::vector<Texture*> foundTexs;
							checkMaterials.push_back(mesh->getMaterial());

							for (auto maps : mesh->getMaterial()->getTextures()) {
								foundTexs.push_back(m_texLoading->findTexture(maps));
							}
							/*for (auto tex : foundTexs) {
								std::cout << "Short path: " << tex->getFilePath() << std::endl;
							}*/

							materialPath.push_back(MaterialPaths{ mesh->getDisplayName(),
							foundTexs[0]->getFilePath(), mesh->getMaterial()->useDiffuseTexture, mesh->getMaterial()->diffuseColor,
							foundTexs[1]->getFilePath(), mesh->getMaterial()->useMetallicTexture, mesh->getMaterial()->metallic,
							foundTexs[2]->getFilePath(), mesh->getMaterial()->useRoughnessTexture, mesh->getMaterial()->roughness,
							foundTexs[3]->getFilePath() });
						}				

						fileMeshes.push_back(FileMeshes{ 
							mesh->getDisplayName(),
							mesh->getPosition(),
							mesh->getScaling(),
							mesh->getRotation(), mesh->getMaterial()->getMaterialIndex() });
						texIndex++;
						std::cout << "Material ID: " << mesh->getMaterial()->getMaterialIndex() << std::endl;
					}
					fileModels.push_back(FileModels{ model->getModelPath(), fileMeshes });
					for (auto mesh : fileMeshes) {
						std::cout << "Path: " << model->getModelPath() << " - Model Name: " << mesh.modelName << std::endl;
					}
					for(auto model : fileModels)
						std::cout << "Model path: " << model.modelPath << std::endl;
				}
				// Create and serialize an object
				std::cout << "Background tex path: " << m_HDRI->getBackgroundTexture()->getFilePath() << std::endl;
				SaveFile original(m_scene->getLights(), materialPath, fileModels, m_HDRI->getBackgroundTexture()->getFilePath(), m_HDRI->getHDRI_Path());
				original.serialize(std::string(ASSET_DIR) + "/Saves/" + saveName + ".bin");
				
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
		if (deferredRendering) {
			glUseProgram(0); // Unbind any active shader
			m_GBuffer->constructDeferredShaders();
			m_GBuffer->deconstructForwardShaders();
			m_SSAO->constructSSAO();
			m_SSAO->constructSSR();
		}
		else if (!deferredRendering) {
			glUseProgram(0); // Unbind any active shader
			m_SSAO->deconstructSSAO();
			m_SSAO->deconstructSSR();
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
					for (auto models : m_scene->getModels()) {
						for (auto meshes : models->getMeshes()) {
							meshes->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
							meshes->setScaling(glm::vec3(1.0f, 1.0f, 1.0f));
						}
					}			

				if (ImGui::Button("Reset rotation"))
					for (auto models : m_scene->getModels()) {
						for (auto meshes : models->getMeshes()) {
							meshes->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
						}
					}

				if (ImGui::Button("Reset scale"))
					for (auto models : m_scene->getModels()) {
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
				for (auto model : m_scene->getModels()) {
					for (size_t i = 0; i < model->getMeshes().size(); i++)
					{
						Mesh* meshes = model->getMeshes()[i];
						if (ImGui::TreeNode(("Mesh " + model->getMeshes()[i]->getDisplayName() + " " + std::to_string(i)).c_str())) // Added an index to the name to avoid duplicates
						{
							//ImGui::Text(model->getMeshes()[i]->getDisplayName().c_str());
							//ImGui::Dummy(ImVec2(0.0f, 7.5f));

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
								for (size_t i = 0; i < m_scene->getMaterials().size(); i++)
								{
									bool isSelected = (m_scene->getMaterials()[i] == currentMat);
									if (ImGui::Selectable(m_scene->getMaterials()[i]->getName().c_str(), isSelected))
									{
										meshes->setMaterial(m_scene->getMaterials()[i]);  // Set the selected material to the mesh
									}
									if (isSelected)
										ImGui::SetItemDefaultFocus();  // Ensure selected item is focused
								}
								ImGui::EndCombo();
							}
							ImGui::TreePop();
						}

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

				if (ImGui::Button("Update model folder")) {
					updateMeshFiles();
				}

				// Load selected HDR file and generate the maps for them
				if (ImGui::Button("Add new mesh")) {
					// Load the selected mesh
					std::string selectedItem = ("/models/" + meshFiles[currentItem]);
					std::vector<Mesh*> newMeshes = m_texLoading->processMeshes(selectedItem);

					m_scene->getModels().push_back(new Model(selectedItem, newMeshes));

					// Add the new mesh to the std::vector
					for (auto& mesh : newMeshes) {
						mesh->setMaterial(m_scene->getMaterials()[0]);
					}

				}

				// For selecting and removing meshes
				if (ImGui::TreeNode("Loaded Meshes"))
				{
					for (size_t i = 0; i < m_scene->getModels().size(); i++) {
						ImGui::Text("Mesh %s", m_scene->getModels()[i]->getModelPath().c_str());

						if (ImGui::TreeNode(("Child Meshes " + std::to_string(i)).c_str())) { // ("str" + str).c_str(), reminder because I'm a troglodyte
							for (auto meshes : m_scene->getModels()[i]->getMeshes()) {
								ImGui::Text("Mesh vertex count: %d", meshes->getVertices());
							}
							ImGui::TreePop();
						}

						if (ImGui::Button(("Remove##" + std::to_string(i)).c_str())) { // Prevent duplicated names (duplicated names have uniform actions for all iterations)
							// Remove mesh from vector and cleanup
							delete m_scene->getModels()[i];
							m_scene->getModels().erase(m_scene->getModels().begin() + i);
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
				ImGui::Text("Amount of existing lamps: %zu", m_scene->getLights().size());

				// Point light addition
				if (ImGui::Button("Add new point light") && m_scene->getLights().size() < 12) {
					m_scene->getLights().push_back(FileLights{ glm::vec3(0.0, 0.0, 2.0), glm::vec3(1.0f, 0.5f, 0.31f), 5.0f });
				}
				else if (m_scene->getLights().size() == 12) {
					ImGui::Text("Maximum amount of lamps reached!!!");
				}

				ImGui::Dummy(ImVec2(5.0f, 5.0f));

				if (ImGui::TreeNode("POINT LAMPS"))
				{
					for (size_t i = 0; i < m_scene->getLights().size(); i++)
					{
						ImGui::PushID(static_cast<int>(i));	// Each control to be unique
						ImGui::Text("Point Light %zu", i);

						ImGui::DragFloat3("Position", glm::value_ptr(m_scene->getLights()[i].pos), 0.1f);
						ImGui::ColorEdit3("Color", glm::value_ptr(m_scene->getLights()[i].color));
						ImGui::InputFloat("Strength", &m_scene->getLights()[i].strength);

						if (ImGui::Button("Erase point light")) {
							m_scene->getLights().erase(m_scene->getLights().begin() + i);
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

				if (deferredRendering) {
					if (ImGui::Checkbox("Lighting Orientation (only for deferred!)", &lightOrientationOn)) {
						if (!m_GBuffer->getLightPass() == 0) {
							shaderSet("worldCoords", lightOrientationOn);
						}
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
						Texture* newBackground = m_texLoading->loadTexture(selectedItem.c_str(), true);
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
				ImGui::Checkbox("Use Normal Texture", &useNormalTexture);
				if (useNormalTexture) {
					if (ImGui::BeginCombo("Normal Texture", materialFileNames[currentItem[3]]))
					{
						displayMatList(3, currentItem, materialFileNames);
						ImGui::EndCombo();
					}
				}

				ImGui::Dummy(ImVec2(0.0, 4.0f));

				ImGui::InputText("Set name for the material", materialName, IM_ARRAYSIZE(materialName));
				if (ImGui::Button("Create a new material"))
				{
					stbi_set_flip_vertically_on_load(false);

					std::string normalMapName = "EmptyNormal.png";
					if (useNormalTexture)
						normalMapName = materialFiles[currentItem[3]];

					Material* newMaterial = m_texLoading->getMaterialMap()[m_texLoading->getMaterialMap().size() + 1] = m_texLoading->checkAndAddMaterial(m_texLoading->loadTextureSet(
						std::string("/textures/" + materialFiles[currentItem[0]]), // Diffuse
						std::string("/textures/" + materialFiles[currentItem[1]]), // Metallic
						std::string("/textures/" + materialFiles[currentItem[2]]), // Roughness
						std::string("/textures/" + normalMapName) // Normal
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