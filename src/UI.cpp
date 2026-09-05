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

	updatePostProcess();
	m_resoManager->updateAllFiles();
}

utils::utils fpsCounter;

UI::~UI() {}

void displayMatList(const int& item, static int currentItem[], std::vector<const char*> materialFileNames) {
	for (std::int8_t i = 0; i < materialFileNames.size(); i++) {
		bool isSelected = (currentItem[item] == i);
		if (ImGui::Selectable(materialFileNames[i], isSelected))
			currentItem[item] = i;		
		if (isSelected)
			ImGui::SetItemDefaultFocus();
	}
}

void UI::renderPostProcessSliders(PostProcess& pp, Shader* forwardShader, Shader* deferredShader) {
	if (ImGui::SliderFloat((pp.name + "Exposure").c_str(), &pp.exposure, 0.0f, 5.0f))
		shaderSetDual((pp.name + "Exposure").c_str(), pp.exposure, forwardShader, deferredShader);

	if (ImGui::SliderFloat((pp.name + "Contrast").c_str(), &pp.contrast, 0.0f, 5.0f))
		shaderSetDual((pp.name + "Contrast").c_str(), pp.contrast, forwardShader, deferredShader);

	if (ImGui::DragFloat3((pp.name + "Hue").c_str(), glm::value_ptr(pp.hue), 0.01f))
		shaderSetDual((pp.name + "Hue").c_str(), pp.hue, forwardShader, deferredShader);

	// Load selected HDR file and generate the maps for them
	if (ImGui::Button(("Reset parameters: " + pp.name).c_str())) {
		pp.exposure = 1.0f;
		pp.contrast = 1.0f;
		pp.hue = glm::vec3(1.0f);

		shaderSetDual((pp.name + "Exposure").c_str(), pp.exposure, forwardShader, deferredShader);
		shaderSetDual((pp.name + "Contrast").c_str(), pp.contrast, forwardShader, deferredShader);
		shaderSetDual((pp.name + "Hue").c_str(), pp.hue, forwardShader, deferredShader);
	}
}

using transformFunction = void (GameObject::*)(const glm::vec3&); // A function

void applyTransform(Mesh* mesh, const std::function<void(GameObject*)>& transformFunc) {
	transformFunc(mesh);
}
void applyTransform(Model* model, const std::function<void(GameObject*)>& transformFunc) {
	for (auto& mesh : model->getMeshes()) {
		transformFunc(mesh);
	}
	transformFunc(model);
}

// Life could be a dream
template<typename T>
void renderDragFloat3(const char* name, T* obj, transformFunction transformFunc, glm::vec3& value) {
	if (ImGui::DragFloat3(name, glm::value_ptr(value), 0.01f)) {
		applyTransform(obj, [&](GameObject* inObj) { (inObj->*transformFunc)(value); });
	}
}

template<typename T>
void renderDragFloat(const char* name, T* obj, transformFunction transformFunc, glm::vec3& value) {
	float totalValue = value.x; // kinda stupid, but ok for now (maybe get the average later)
	if (ImGui::DragFloat(name, &totalValue, 0.01f)) {
		value = glm::vec3(totalValue);
		applyTransform(obj, [&](GameObject* inObj) { (inObj->*transformFunc)(value); });
	}
}

template<typename T> void UI::meshTransformationUI(T* mesh, glm::vec3 values[3],  std::string targetName) {
	renderDragFloat3((targetName + " Position").c_str(), mesh, &GameObject::setPosition, values[0]);

	// Control for scale
	static bool scaleLock = false;
	ImGui::Checkbox((targetName + " Scalelock").c_str(), &scaleLock);

	if (!scaleLock)
		renderDragFloat3((targetName + " Scale").c_str(), mesh, &GameObject::setScaling, values[1]); // Set indiviudal XYZ scaling
	else
		renderDragFloat((targetName + " Scale").c_str(), mesh, &GameObject::setScaling, values[1]);

	renderDragFloat3((targetName + " Rotation").c_str(), mesh, &GameObject::setRotation, values[2]);
}

void UI::renderMeshTreeNode(Model* model, std::uint16_t nameIndex) {
	if (ImGui::TreeNode((model->getModelPath() + ": " + std::to_string(nameIndex)).c_str())) {
		glm::vec3 psrModel[3] = { model->getPosition(), model->getScaling(), model->getRotation() };
		meshTransformationUI(model, psrModel, "Model");

		for (size_t i = 0; i < model->getMeshes().size(); i++) {
			Mesh* meshes = model->getMeshes()[i];
			if (ImGui::TreeNode(("Mesh " + model->getMeshes()[i]->getDisplayName() + " " + std::to_string(i)).c_str())) // Added an index to the name to avoid duplicates
			{
				glm::vec3 psrMesh[3] = { meshes->getPosition(), meshes->getScaling(), meshes->getRotation() };
				meshTransformationUI(meshes, psrMesh, "Mesh");
				changeMaterial(meshes);

				ImGui::TreePop();
			}

		}
		ImGui::TreePop();
	}
}

void UI::changeMaterial(Mesh* mesh) {
	// Control for material
	Material* currentMat = mesh->getMaterial();
	const char* changeMat = currentMat ? currentMat->getName().c_str() : "None"; // get a const char

	if (ImGui::BeginCombo("Material", changeMat))  // Combo box to choose material
	{
		for (size_t i = 0; i < m_resoManager->getScene()->getMaterials().size(); i++)
		{
			bool isSelected = (m_resoManager->getScene()->getMaterials()[i] == currentMat);
			if (ImGui::Selectable(m_resoManager->getScene()->getMaterials()[i]->getName().c_str(), isSelected)) {
				mesh->setMaterial(m_resoManager->getScene()->getMaterials()[i]);  // Set the selected material to the mesh
				m_resoManager->getScene()->updateMeshList();
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();  // Ensure selected item is focused
		}
		ImGui::EndCombo();
	}
}

void UI::useAutomaticTextureFinding(const static std::uint8_t& currentItem) {
	// Section for the targeted file searching from a folder
	if (ImGui::Checkbox("Use a Specific Folder Search", &useFolderFiltering))
		m_resoManager->setFolderSearchPath(defaultFolderPath); // Let it be default value

	if (useFolderFiltering) {
		static int currentFolder = 0;
		std::vector<std::string>& m_folderNames = m_resoManager->m_folderNames;

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
		std::string selectedItem = ("/models/" + m_resoManager->meshFileNames[currentItem]);
		std::vector<Mesh*> newMeshes = m_resoManager->processMeshes(selectedItem, true);

		m_resoManager->getScene()->getModels().push_back(new Model(selectedItem, newMeshes));
		m_resoManager->getScene()->updateMeshList();
	}
}

void UI:: useRegularModelLoading(const static std::uint8_t currentItem) {
	// Load selected HDR file and generate the maps for them
	if (ImGui::Button("Add new mesh")) {
		// Load the selected mesh
		std::string selectedItem = ("/models/" + m_resoManager->meshFileNames[currentItem]);
		std::vector<Mesh*> newMeshes = m_resoManager->processMeshes(selectedItem);

		m_resoManager->getScene()->getModels().push_back(new Model(selectedItem, newMeshes));
		m_resoManager->getScene()->updateMeshList();
	}
}

void createComboBox(const char* comboName, std::vector<const char*>& materialFileNames, static int currentItem[], const int& boxIndex) {
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

void UI::renderMaterialOptions(SettingsMaterial& SetMat, static int currentItem[]) {
	// This is probably quite useless, should probably just use std::string instead and not convert these to c_str() everytime
	std::vector<const char*> materialFileNames;
	for (const auto& file : m_resoManager->m_materialFileNames) {
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
		createComboBox("Opacity Texture", materialFileNames, currentItem, 4);
	}
	else {
		ImGui::SliderFloat("Opacity Value", &SetMat.opacity, 0.0f, 1.0f);
	}

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	// ComboBox for Normal (only texture currently)
	ImGui::Checkbox("Use Normal Texture", &SetMat.useNormalTexture);
	if (SetMat.useNormalTexture) {
		createComboBox("Normal Texture", materialFileNames, currentItem, 5);
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
				for (auto files : m_resoManager->m_saveFiles) {
					if (ImGui::MenuItem(files.c_str())) {
						std::cout << "Opened " + std::string(files) + "\n";

						// Clean up the whole scene
						m_resoManager->cleanResourceManager();
						m_resoManager->fileLoad(files);
					}
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Save")) {
				std::cout << "Saved" << std::endl;
				m_resoManager->fileSave(saveName);	
				m_resoManager->updateFiles(m_resoManager->m_saveFiles, "Saves/", [this](const std::string& path) { return m_resoManager->FileSystem(path); });
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

	shaderBind(); // Bind the current shader

	if (ImGui::Checkbox("Deferred Rendering", &m_GBuffer->getRenderMode())) {
		if (m_GBuffer->getRenderMode())
			m_SSAO->constructDeferredRendering();
		else if (!m_GBuffer->getRenderMode())
			m_SSAO->constructForwardRendering();
	}

	if(ImGui::Button("Set Resolution")) {
		m_GBuffer->updateResolution();
		m_GBuffer->getRenderMode() ? m_SSAO->recreateColorBuffer() : m_SSAO->recreateGaussianBlur();
	}

	if (ImGui::Checkbox("Wireframe mode", &m_wireFrame)) {
		m_wireFrame ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	ImGui::Dummy(ImVec2(0.0f, 2.0f));
	ImGui::Text("'Refetch Files' -button updates the available files found in the asset-folder");
	if (ImGui::Button("Refetch Files")) {
		m_resoManager->updateAllFiles();
	}
	ImGui::Dummy(ImVec2(0.0f, 2.0f));

	if (ImGui::BeginTabBar("MyTabs"))
	{
		if (ImGui::BeginTabItem("MESH CONTROLS"))
		{
			// Reset options for transformations
			if (ImGui::TreeNode("RESETS"))
			{
				if (ImGui::Button("Reset all the transforms")) {
					//m_resoManager->transformOperation(glm::vec3(0.0f, 0.0f, 0.0f), GameObject::setRotation);
					//m_resoManager->transformOperation(glm::vec3(1.0f, 1.0f, 1.0f), setScale);
				}

				if (ImGui::Button("Reset rotation"))
					//m_resoManager->transformOperation(glm::vec3(0.0f, 0.0f, 0.0f), setRotation);

				if (ImGui::Button("Reset scale"))
					//m_resoManager->transformOperation(glm::vec3(1.0f, 1.0f, 1.0f), );

				ImGui::TreePop();
			}
			ImGui::Separator();

			// Control transformations
			if (ImGui::TreeNode("TRANSFORM/MATERIAL MESHES"))
			{
				ImGui::Text("Below is all the meshes and their transforms");
				std::uint16_t nameIndex = 0;
				for (auto model : m_resoManager->getScene()->getModels()) {

					renderMeshTreeNode(model, nameIndex);
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

				static std::uint8_t currentItem = 0;
				std::vector<std::string>& meshFileNames = m_resoManager->meshFileNames;
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

				ImGui::Checkbox("Use automatic texture finding", &m_useAutomaticTextures);
				m_useAutomaticTextures ? useAutomaticTextureFinding(currentItem) : useRegularModelLoading(currentItem);

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
							m_resoManager->removeModelBySelection(i); // Remove mesh from vector and cleanup
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
				std::vector<std::string>& hdrFileNames = m_resoManager->hdrFileNames;

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

				if (m_GBuffer->getRenderMode()) {
					if (ImGui::Checkbox("Lighting Orientation (only for deferred!)", &lightOrientationOn)) {
						if (!m_GBuffer->getLightPass() == 0) {
							shaderSet("worldCoords", lightOrientationOn);
						}
					}
				}				

				renderPostProcessSliders(pp_HDRI, m_GBuffer->getForwardShader(), m_GBuffer->getCompositeShader());
				ImGui::Dummy(ImVec2(0.0f, 10.0f));
				renderPostProcessSliders(pp_model, m_GBuffer->getForwardShader(), m_GBuffer->getCompositeShader());

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
					for (const auto& file : backgroundFiles) {
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

					renderPostProcessSliders(pp_background, m_backImage, 0);

					if (ImGui::Button("Set Background Texture")) {
						// Clean up background texture
						m_HDRI->cleanBackgroundTexture();
						std::string selectedItem = "/backgrounds/" + backgroundFiles[currentBackground];
						Texture* newBackground = m_resoManager->loadTexture(selectedItem.c_str(), true);
						m_HDRI->setBackgroundTexture(newBackground);
					}
				}

				ImGui::TreePop();
			}
			ImGui::Separator();

			ImGui::EndTabItem();
		}
		if (m_GBuffer->getRenderMode()) {
			if (ImGui::BeginTabItem("SCREEN-SPACE"))
			{
				if (ImGui::TreeNode("Bloom"))
				{
					BLOOM_SETTINGS& bloom = m_SSAO->getBloom_Settings();
					if (ImGui::Checkbox("Use Bloom", &bloom.useBloom))
						m_SSAO->recreateColorBuffer();
					ImGui::InputInt("Gaussian Blur Repetitions", &bloom.amount);
					if (ImGui::SliderInt("Bloom Distance", &bloom.distance, 1, 5)) // Min: 1, Max: 5... currently there's a fixed array of five gaussian weights
						bloom.dirty = true;

					ImGui::TreePop();
				}
				ImGui::Separator();

				if (ImGui::TreeNode("SSAO"))
				{			
					SSAO_SETTINGS& ssao = m_SSAO->getSSAO_Settings();
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
					SSR_SETTINGS& ssr = m_SSAO->getSSR_Settings();
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
				static int currentItem[6] = { 0, 0, 0, 0, 0, 0 }; // One for each texture type
				static char materialName[128] = ""; // Input field for material name

				renderMaterialOptions(m_resoManager->getSettingsCreate(), currentItem);

				ImGui::InputText("Set name for the material", materialName, IM_ARRAYSIZE(materialName));
				if (ImGui::Button("Create a new material")) {
					stbi_set_flip_vertically_on_load(false);

					// In case, where the texture is not used ("" is handled as no texture)
					std::string usePaths[6];
					m_resoManager->findMaterialPaths(usePaths, m_resoManager->getSettingsCreate(), m_resoManager->m_materialFileNames, currentItem); // Edit SettingsCreate

					// Set the default material
					m_resoManager->createMaterial(m_resoManager->createMaterialPaths(materialName, usePaths, m_resoManager->getSettingsCreate()));
					m_resoManager->getScene()->updateMeshList();

					for (const auto& texture : m_resoManager->getTrackedTextures())
						std::cout << "Current textures: " << texture.first << std::endl;

				}

				ImGui::TreePop();
			}

			ImGui::Separator();

			if (ImGui::TreeNode("EDIT MATERIALS"))
			{
				static int select = 0;
				static int comboBoxSelection[6] = { 0, 0, 0, 0, 0, 0 }; // One for each texture type
				const std::int8_t comboBoxSize = sizeof(comboBoxSelection) / sizeof(comboBoxSelection[0]);

				std::vector<Material*>& materials = m_resoManager->getScene()->getMaterials();

				if (!materials.empty()) {
					const char* materialName = materials[select] ? materials[select]->getName().c_str() : "None"; // transform string into c_str() or set name to "None"

					if (ImGui::BeginCombo("Material", materialName))
					{
						for (size_t i = 0; i < materials.size(); i++) {
							bool isSelected = (materials[i] == materials[select]);
							if (ImGui::Selectable(m_resoManager->getScene()->getMaterials()[i]->getName().c_str(), isSelected)) {
								select = i; // Set the selected index
								m_resoManager->findComboBoxMaterials(materials[select], comboBoxSize, comboBoxSelection);
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();  // Ensure selected item is focused
						}
						ImGui::EndCombo();
					}

					renderMaterialOptions(m_resoManager->getSettingsEdit(), comboBoxSelection);

					if (ImGui::Button("Apply edited material")) {
						m_resoManager->applyEditedMaterial(materials[select], comboBoxSelection);
					}

					ImGui::Dummy(ImVec2(0.0f, 4.0f));

					ImGui::Text("This removes the selected material");
					if (ImGui::Button("Remove selected material")) {
						m_resoManager->removeMaterialBySelection(select);
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