#include <iostream>
#include <string>
#include <chrono>
#include <cmath>
#include <array>
#include <limits>
#include <tuple>
#include <vector>
#include <thread>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "app_utility.h"
#include "shader.h"
#include "saved_view.h"

static GLFWwindow* window;
static Shader shader;
static int windowWidth = 1080;
static int windowHeight = 720;
static long double zoomScale = 3.5L; //1.7e-10;
static long double realPartStart = -2.5L; //-0.04144230656908739;
static long double imagPartStart = -1.75L; //1.48014290228390966;
static constexpr long double ZOOM_STEP = 1.1L;

static std::array<float, 25> lastFrameDeltas;
static std::size_t lastFrameArrayIndex = 0;
static bool autoMaxIterations = true;
static int maxIterations = 300;
static bool ImGuiEnabled = true;


// * HELPER FUNCTIONS

static int getMaxIterations() {
	//int zoomCount = -std::log(zoomScale / 3.5) / std::log(ZOOM_STEP); // how often you have zoomed in

	if (autoMaxIterations) {
		maxIterations = 400 + 100L * -std::log10(zoomScale);
		if (maxIterations < 200)
			maxIterations = 200;
		else if (maxIterations > 4000)
			maxIterations = 4000;
	}
	return maxIterations;
}

static float calcFPSAverage() {
	float average = 0.0f;
	for (float value : lastFrameDeltas)
		average += value;
	return average / lastFrameDeltas.size();
}

static ComplexNum getNumberAtPos(double x, double y) {
	long double real = zoomScale * (x + 0.5) / windowWidth + realPartStart;
    long double imag = (zoomScale * (y + 0.5) + imagPartStart * windowHeight) / windowWidth;

	return {real, imag};
}

static ComplexNum getNumberAtCursor() {
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	return getNumberAtPos(mouseX, mouseY);
}

// * FUNCTIONS

static void zoom(long double factor) {
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	realPartStart += (1.0L - factor) * zoomScale / windowWidth * mouseX;
	imagPartStart += (1.0L - factor) * zoomScale / windowHeight * ((long double)windowHeight - mouseY);

	zoomScale *= factor;
}

static void jumpToView(const SavedView& savedView) {
	zoomScale = savedView.getZoomScale();
	realPartStart = savedView.getStartNum().first;
	imagPartStart = savedView.getStartNum().second;
}

static void ImGuiFrame(bool& showImGuiWindow) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("Options", &showImGuiWindow, ImGuiWindowFlags_NoCollapse)) {	// Create a window and append into it.
		ImGui::SetWindowSize({0,0}, ImGuiCond_FirstUseEver); // set window to fit contents when first creating it (ImGui saves position bewteen sessions)

		if (ImGui::BeginTabBar("#idTabBar"))
		{
			if (ImGui::BeginTabItem("Info"))
			{
				if (ImGui::SliderInt("Max iterations", &maxIterations, 1, 8000))
					autoMaxIterations = false;
				ImGui::Checkbox("auto max iterations", &autoMaxIterations);

				ImGui::Text("Color: ");
				ImGui::SameLine();
				if (ImGui::SmallButton("RGB"))
					shader.mandelRecompileWithColor(0);
				ImGui::SameLine();
				if (ImGui::SmallButton("Black/White"))
					shader.mandelRecompileWithColor(1);
				ImGui::SameLine();
				if (ImGui::SmallButton("Glowing"))
					shader.mandelRecompileWithColor(2);

				//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::Text("%.1f fps", calcFPSAverage());
				ImGui::Text("Zoom: %.1Le", zoomScale);
				auto [real, imag] = getNumberAtCursor();
				ImGui::Text("Cursor: %.10Lf + %.10Lf i", real, imag);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Saved views"))
			{
				// Button to save current view
				if (ImGui::Button("Save current view"))
					SavedView::saveNew(zoomScale, {realPartStart, imagPartStart});

				for (SavedView& savedView : SavedView::allViews) {
					ImGui::PushID(savedView.getImGuiIDs()[0]);
					if (ImGui::SmallButton("View")) {
						jumpToView(savedView);
					}
					ImGui::PopID();
					ImGui::SameLine();
					ImGui::Text("%s", savedView.getName().c_str());
					ImGui::SameLine();

					static int editSavedViewID = -1;
					ImGui::PushID(savedView.getImGuiIDs()[1]);
					static char buffer[50];
					if (ImGui::SmallButton("Edit")) {
						editSavedViewID = savedView.getImGuiIDs()[0];
						copyStringToBuffer(savedView.getName(), buffer, 50);
					}
					ImGui::PopID();

					if (editSavedViewID == savedView.getImGuiIDs()[0]) {
						ImGui::OpenPopup("Edit saved view");
						ImGui::BeginPopup("Edit saved view"); // TODO https://github.com/ocornut/imgui/issues/2495
						ImGui::SetWindowSize({150, 0}, ImGuiCond_Once);
						if (ImGui::InputTextWithHint("Name", "Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
							savedView.setName(buffer);
							editSavedViewID = -1;
							copyStringToBuffer("", buffer, 2); // Reset buffer to be empty for next input
						}
						// TODO maybe add options to adjust zoom and start number values
						ImGui::EndPopup();
					}

					ImGui::SameLine();
					ImGui::PushID(savedView.getImGuiIDs()[2]);
					if (ImGui::SmallButton("Delete"))
						SavedView::removeSavedView(savedView);
					ImGui::PopID();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Advanced"))
			{
				ImGui::Text("Start real:\t%.25Lf", realPartStart);
				ImGui::Text("Start imag:\t%.25Lf", imagPartStart);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Help"))
			{
				ImGui::Text("Press Ctrl + Enter to toggle GUI");
				ImGui::Text("Press Ctrl + Esc or Pause to exit the app");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}


// * CALLBACK FUNCTIONS

static void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	realPartStart = (0.5l * zoomScale * (1.0l / windowWidth - 1.0l / width)) + realPartStart;
	imagPartStart = ((long double)width / height) * ((zoomScale * (0.5l * windowHeight + 0.5l) + imagPartStart * windowHeight) / windowWidth) - (0.5l * zoomScale * (1.0l + 1.0l / height));
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);
}  

static void debugCallbackOpenGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	std::cout << "OpenGL debug: " << "source: " << source << ", type: " << type << ", id: " << id << ", severity: " << severity << std::endl
		<< message << std::endl;
}

static void errorCallbackGLFW(int error, const char* description) {
	std::cout << "GLFW error: (" << error << "): " << description << std::endl; 
}

static void mouseScrollCallbackGLFW(GLFWwindow* window, double xOffset, double yOffset) {
	if (yOffset == 1.0)
		zoom(1 / ZOOM_STEP);
	else if (yOffset == -1.0)
		zoom(ZOOM_STEP);
}

static void keyCallbackGLFW(GLFWwindow* window, int key, int scancode, int action, int mods) {
	switch (key) {
		case GLFW_KEY_PAUSE:
			if (action == GLFW_PRESS)
				glfwSetWindowShouldClose(window, true);
			break;

		case GLFW_KEY_ESCAPE:
			if (action == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				glfwSetWindowShouldClose(window, true);
			break;

		case GLFW_KEY_ENTER:
			if (action == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				ImGuiEnabled = !ImGuiEnabled;
			break;
	}
}


// * INIT FUNCTIONS

static bool initGLFW() {
	glfwSetErrorCallback(errorCallbackGLFW);

	if (glfwInit() == GL_FALSE) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return false;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create window and context
	window = glfwCreateWindow(windowWidth, windowHeight, "Mandelbrot", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // enable vsync

	// set input callbacks
	glfwSetFramebufferSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallbackGLFW);
	glfwSetScrollCallback(window, mouseScrollCallbackGLFW);

	return true;
}

static bool initGlad() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// Print OpengL version and GPU
	std::cout << glGetString(GL_VERSION) << std::endl
		<< glGetString(GL_VENDOR) << ", " << glGetString(GL_RENDERER) << std::endl;

	// The place where openGL draws
	glViewport(0, 0, windowWidth, windowHeight);

	// Add debug callback for opengl
	glDebugMessageCallback(debugCallbackOpenGL, nullptr);

	return true;
}

static void initImGui() {
	// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
	//ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::BuildmaxDigits()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

	ImFont* font = io.Fonts->AddFontFromFileTTF((AppRootDir + "res/ImGuiFonts/Roboto-Medium.ttf").c_str(), 15.0f);
	if (font == nullptr)
		std::cout << "Error: Font for ImGui could not be loaded" << std::endl;
}


// * MAIN FUNCTION

int main()
{
	SavedView::initFromFile();

	if (!initGLFW())
		return -1;
	if (!initGlad())
		return -1;
	initImGui();

	shader = {AppRootDir + "res/vertex_shader.glsl", AppRootDir + "res/fragment_shader.glsl", true, false}; // Compile and link shader, but keep sources, ...

	float vertices[] = {
		-1.0f, -1.0f,	// bottom left
		 1.0f, -1.0f,	// bottom right
		 1.0f,  1.0f,	// top right
		-1.0f,  1.0f,	// top left
	};
	unsigned int indices[] = {
		0, 1, 2,	// first triangle
		0, 2, 3,	// second triangle
	};

	// vertex buffer
	unsigned int vertexBuffer;
	glGenBuffers(1, &vertexBuffer);

	// element buffer
	unsigned int elementBuffer;
	glGenBuffers(1, &elementBuffer);

	// vertex array object
	unsigned int vertexArray;
	glGenVertexArrays(1, &vertexArray);

	// init vertex array, vertex buffer and element buffer together
	glBindVertexArray(vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// unbind vertex buffer and vertex array (not necessary)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		using timePoint = decltype(std::chrono::high_resolution_clock::now());
		timePoint startTime;

		static bool showImGuiWindow = true;
		if (ImGuiEnabled) {
			startTime = std::chrono::high_resolution_clock::now();
			ImGuiFrame(showImGuiWindow);
		}

		// use program
		shader.use();

		shader.setVec2UInt("windowSize", windowWidth, windowHeight);
		shader.setDouble("zoomScale", zoomScale);
		shader.setVec2Double("numberStart", realPartStart, imagPartStart);
		shader.setUInt("maxIterations", getMaxIterations());
	
		if (ImGuiEnabled)
			ImGui::Render();

		// clear screen
		glClearColor(0.0f, 0.05f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// draw
		glBindVertexArray(vertexArray);
		glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, (void*)0);

		if (ImGuiEnabled)
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// check and call events ans swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		if (ImGuiEnabled) {
			timePoint endTime = std::chrono::high_resolution_clock::now();
			auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
			lastFrameDeltas[lastFrameArrayIndex++] = (1000.0f / delta.count());
			if (lastFrameArrayIndex == lastFrameDeltas.size())
				lastFrameArrayIndex = 0;
		}

		if (!showImGuiWindow) {
			ImGuiEnabled = false;
			showImGuiWindow = true;
		}
	}

	// delete al resources (not necessary)
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(1, &vertexBuffer);
	shader.clean();
	shader.deleteProgram();

	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}
