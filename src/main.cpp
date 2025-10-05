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
#include "screenshot.h"

static GLFWwindow* window;
static Shader shader;
static int windowWidth = 1080;
static int windowHeight = 720;
static long double zoomScale = 3.5L; //1.7e-10;
static long double centerX = -2.5L; //-0.04144230656908739;
static long double centerY = -1.75L; //1.48014290228390966;
static unsigned int vertexArray = 0; // VAO 
static bool zoomingIn = false; // e.g. if Ctrl + Plus is pressed, this is true
static bool zoomingOut = false;
static bool zoomingInSlow = false;
static bool zoomingOutSlow = false;
static constexpr long double ZOOM_STEP = 1.1L; // single zoom step (multiplicative)
static constexpr long double ZOOM_STEP_SMALL = 1.01L;
static constexpr double ZOOM_PER_SECOND = 3.0; // continuous zoom (multiplicative)
static constexpr double SLOW_ZOOM_PER_SECOND = 1.3;

static std::array<float, 25> lastFrameDeltas;
static std::size_t lastFrameArrayIndex = 0;
static bool autoMaxIterations = false;

// RK45 Paramters
static int maxIterations = 10'000;
static uint maxSameSteps = 30u;
static float minStepSize = 1e-12f;
static float atolExponent = -5.0f; // corresponds to 1e-5
static float rtolExponent = -5.0f; // corresponds to 1e-5

// Double Pendulum Parameters
static float simulationEndTime = 3.0f;
static float v1Start = 0.0f;
static float v2Start = 0.0f;
static float weightConstant = 9.81f;
static float length1 = 1.0f;
static float length2 = 1.0f;
static float mass1 = 1.0f;
static float mass2 = 1.0f;

// N-Body Problem Parameters
// constexpr const uint N = 3u;
// constexpr const uint D = 3u;
// static float simulationEndTime = 0.0f;
// static float masses[N] = { 1.0f, 0.9f, 1.2f };

static bool ImGuiEnabled = true;

// static const char* FLOW_COLOR_TYPE = "FLOW_COLOR_TYPE";
// static const char* CODE_DIVERGENCE_CRITERION = "CODE_DIVERGENCE_CRITERION";
// static const char* CODE_CALCULATE_NEXT_SEQUENCE_TERM = "CODE_CALCULATE_NEXT_SEQUENCE_TERM";

// static const char* DEFAULT_FLOW_COLOR_TYPE = "3";
// static char codeDivergenceCriterion[1000] = "real*real + imag*imag > 4";
// static char codeCalculateNextSequenceTerm[2000] = "real = real*real - imag*imag + startReal;\nimag = 2 * realTemp * imag + startImag;";


// * HELPER FUNCTIONS

static int getMaxIterations() {
	//int zoomCount = -std::log(zoomScale / 3.5) / std::log(ZOOM_STEP); // how often you have zoomed in

	if (autoMaxIterations) {
		maxIterations = static_cast<int>(400 + 100L * -std::log10(zoomScale));
		if (maxIterations < 200)
			maxIterations = 200;
		else if (maxIterations > 4000)
			maxIterations = 4000;
	}
	return maxIterations;
}

static void stopContinuousZooming() {
	zoomingIn = false;
	zoomingOut = false;
	zoomingInSlow = false;
	zoomingOutSlow = false;
}

static float calcFPSAverage() {
	float average = 0.0f;
	for (float value : lastFrameDeltas)
		average += value;
	return average / lastFrameDeltas.size();
}

// * FUNCTIONS
/** A measure of how big the window is. Could be anything, just has to be the same everywhere including in the shader */
static inline long double getWindowSize() {
	return std::min(windowWidth, windowHeight);
}

static ComplexNum getNumberAtPos(double x, double y) {
	long double windowSize = getWindowSize();
	long double real = (zoomScale / windowSize) * (x + 0.5L - windowWidth / 2.0L) + centerX;
    long double imag = (zoomScale / windowSize) * (y + 0.5L - windowHeight / 2.0L) + centerY;
	return {real, imag};
}

static ComplexNum getNumberAtCursor() {
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	mouseY = windowHeight - mouseY;
	return getNumberAtPos(mouseX, mouseY);
}

static void zoom(long double factor) {
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	mouseY = windowHeight - mouseY;

	auto windowSize = getWindowSize();
	centerX += ((1.0L - factor) * zoomScale / windowSize) * (static_cast<long double>(mouseX) + 0.5L - windowWidth / 2.0L);
	centerY += ((1.0L - factor) * zoomScale / windowSize) * (static_cast<long double>(mouseY) + 0.5L - windowHeight / 2.0);
	zoomScale *= factor;

	shader.setDouble("zoomScale", static_cast<double>(zoomScale));
	shader.setVec2Double("center", { centerX, centerY });
}

static void jumpToView(const SavedView& savedView) {
	zoomScale = savedView.getZoomScale();
	centerX = savedView.getCenter().first;
	centerY = savedView.getCenter().second;

	shader.setDouble("zoomScale", static_cast<double>(zoomScale));
	shader.setVec2Double("center", { centerX, centerY });
}

static void ImGuiFrame(bool& showImGuiWindow) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("Options", &showImGuiWindow, ImGuiWindowFlags_NoCollapse)) {	// Create a window and append into it.
		ImGui::SetWindowSize({0,0}, ImGuiCond_FirstUseEver); // set window to fit contents when first creating it (ImGui saves position between sessions)

		if (ImGui::BeginTabBar("#idTabBar"))
		{
			if (ImGui::BeginTabItem("Info"))
			{
				ImGui::Text("RK45 Controls: ");
				if (ImGui::SliderInt("Max iterations", &maxIterations, 1, 100'000))
					autoMaxIterations = false;
				//ImGui::Checkbox("Auto max iterations", &autoMaxIterations);

				if (ImGui::SliderFloat("Absolute Tolerance Exponent (10^x)", &atolExponent, -14.0, 2.0))
					shader.setFloat("atol", std::pow(10.0f, atolExponent));

				if (ImGui::SliderFloat("Relative Tolerance Exponent (10^x)", &rtolExponent, -14.0, 2.0))
					shader.setFloat("rtol", std::pow(10.0f, rtolExponent));


				ImGui::Text("Color: ");
				// ImGui::SameLine();
				// if (ImGui::SmallButton("RGB")) {
				// 	shader.define(FLOW_COLOR_TYPE, "0");
				// 	shader.recompile();
				// }
				// ImGui::SameLine();
				// if (ImGui::SmallButton("Smooth")) {
				// 	shader.define(FLOW_COLOR_TYPE, "3");
				// 	shader.recompile();
				// }
				
				// // new line
				// if (ImGui::SmallButton("Black/White")) {
				// 	shader.define(FLOW_COLOR_TYPE, "1");
				// 	shader.recompile();
				// }
				// ImGui::SameLine();
				// if (ImGui::SmallButton("Glowing")) {
				// 	shader.define(FLOW_COLOR_TYPE, "2");
				// 	shader.recompile();
				// }

				// static int colorAccuracy = 10;
				// if (ImGui::DragInt("Color accuracy", &colorAccuracy, 1.0f, 1, 1'000))
				// 	shader.setUInt("colorAccuracy", (uint)colorAccuracy);
				// ImGui::NewLine();

				// Sequence
				// ImGui::Text("Sequence code (1. Divergence criterion, 2. Code to calculate next term in sequence)");
    			// ImGui::InputTextMultiline("1", codeDivergenceCriterion, IM_ARRAYSIZE(codeDivergenceCriterion),
				// 	ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2.5f)
				// );
				// ImGui::InputTextMultiline("2", codeCalculateNextSequenceTerm, IM_ARRAYSIZE(codeCalculateNextSequenceTerm),
				// 	ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3.5f)
				// );
				// if (ImGui::SmallButton("Apply")) {
				// 	shader.define(CODE_DIVERGENCE_CRITERION, std::string(codeDivergenceCriterion));
				// 	shader.define(CODE_CALCULATE_NEXT_SEQUENCE_TERM, std::string(codeCalculateNextSequenceTerm));
				// 	shader.recompile();
				// }
				// ImGui::NewLine();

				ImGui::Text("Double Pendulum Controls: ");
				if (ImGui::SliderFloat("End-Time", &simulationEndTime, 0.0, 10.0)) {
					shader.setFloat("t_end", simulationEndTime);
				}
				if (ImGui::SliderFloat("v1-Start", &v1Start, -1.0f, +1.0f)) {
					shader.setFloat("v1_start", v1Start);
				}
				if (ImGui::SliderFloat("v2-Start", &v2Start, -1.0f, +1.0f)) {
					shader.setFloat("v2_start", v2Start);
				}
				if (ImGui::SliderFloat("g", &weightConstant, -5.0, 40.0)) {
					shader.setFloat("g", weightConstant);
				}
				if (ImGui::SliderFloat("l1", &length1, -1.0, 8.0)) {
					shader.setFloat("l1", length1);
				}
				if (ImGui::SliderFloat("l2", &length2, -1.0, 8.0)) {
					shader.setFloat("l2", length2);
				}
				if (ImGui::SliderFloat("m1", &mass1, -2.0, 8.0)) {
					shader.setFloat("m1", mass1);
				}
				if (ImGui::SliderFloat("m2", &mass2, -2.0, 8.0)) {
					shader.setFloat("m2", mass2);
				}

				// ImGui::Text("N-Body Problem Controls: ");
				// if (ImGui::SliderFloat("End-Time", &simulationEndTime, 0.0f, 0.1f)) {
				// 	shader.setFloat("t_end", simulationEndTime);
				// }
				// if (ImGui::SliderFloat("mass1", &masses[0], 0.0f, +5.0f)) {
				// 	shader.setFloatArray("m", masses, N);
				// }
				// if (ImGui::SliderFloat("mass2", &masses[1], 0.0f, +5.0f)) {
				// 	shader.setFloatArray("m", masses, N);
				// }
				// if (ImGui::SliderFloat("mass3", &masses[2], 0.0f, +5.0f)) {
				// 	shader.setFloatArray("m", masses, N);
				// }

				// Status info
				//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::Text("%.1f fps", static_cast<double>(calcFPSAverage()));
				ImGui::Text("Zoom: %.1Le", zoomScale);
				auto [real, imag] = getNumberAtCursor();
				ImGui::Text("Cursor: %.10Lf + %.10Lf i", real, imag);
				ImGui::Text("Center: %.10Lf + %.10LF i", centerX, centerY);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Saved views"))
			{
				// Button to save current view
				if (ImGui::Button("Save current view"))
					SavedView::saveNew(zoomScale, {centerX, centerY});

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
			if (ImGui::BeginTabItem("Screenshots")) {
				static char screenshotFilename[128] = "screenshot.png";
				static int captureWidth  = 1920;
				static int captureHeight = 1080;
				static int maxTileSize = 2048;
				static float thresholdFactor = 0.01f;

				ImGui::InputText("Filename", screenshotFilename, sizeof(screenshotFilename));
				ImGui::InputInt("Width", &captureWidth);
				ImGui::InputInt("Height", &captureHeight);
				ImGui::InputInt("Max Tile Size", &maxTileSize);
				ImGui::SliderFloat("Threshold Factor", &thresholdFactor, 1e-10f, 500.0f);

				if (ImGui::Button("Take Screenshot")) {
					// Set low tolerance for rk45
					shader.setFloat("atol", 1e-9f);
					shader.setFloat("rtol", 1e-9f);
					shader.setUInt("MAX_SAME_STEPS", 50u);
					shader.setFloat("MIN_TAU", 1e-25f);
					shader.setFloat("thresholdFactor", thresholdFactor);

					// Take the screenshot
					takeScreenshot(
						screenshotFilename,
						static_cast<size_t>(std::max(captureWidth, 0)),
						static_cast<size_t>(std::max(captureHeight, 0)),
						shader,
						vertexArray,
						zoomScale,
						centerX,
						centerY,
						100'000, // real-time-app uses getMaxIterations()
						static_cast<size_t>(maxTileSize)
					);

					// Reset everything that was changed before calling takeScreenshot
					shader.setFloat("atol", std::pow(10.0f, atolExponent));
					shader.setFloat("rtol", std::pow(10.0f, rtolExponent));
					shader.setUInt("MAX_SAME_STEPS", maxSameSteps);
					shader.setFloat("MIN_TAU", minStepSize);
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Help"))
			{
				ImGui::Text("Press Ctrl + Enter to toggle GUI");
				ImGui::Text("Press Ctrl + Esc or Pause to exit the app");
				ImGui::Text("Press Ctrl + Plus/Minus to zoom in/out (numpad works too)");
				ImGui::Text("Use Mouse Wheel to zoom in/out");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}


// * CALLBACK FUNCTIONS

static void windowResizeCallback(GLFWwindow* _window, int width, int height) {
	(void)_window; // suppress unused warning
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);

	shader.setVec2UInt("windowSize", { windowWidth, windowHeight });
}  

static void debugCallbackOpenGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	(void)length;     // suppress unused warning
	(void)userParam; // suppress unused warning
	std::cout << "OpenGL debug: " << "source: " << source << ", type: " << type << ", id: " << id << ", severity: " << severity << std::endl
		<< message << std::endl;
}

static void errorCallbackGLFW(int error, const char* description) {
	std::cout << "GLFW error: (" << error << "): " << description << std::endl; 
}

static void mouseScrollCallbackGLFW(GLFWwindow* _window, double xOffset, double yOffset) {
	(void)_window; // suppress unused warning
	(void)xOffset; // suppress unused warning

	// GLFW_KEY_LEFT_SHIFT or GLFW_KEY_RIGHT_SHIFT
	bool shiftPressed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		|| (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

	if (yOffset == 1.0) {
		if (!shiftPressed) {
			zoom(1.0 / ZOOM_STEP);
		} else {
			zoom(1.0 / ZOOM_STEP_SMALL);
		}
	}
	else if (yOffset == -1.0) {
		if (!shiftPressed) {
			zoom(ZOOM_STEP);
		} else {
			zoom(ZOOM_STEP_SMALL);
		}
	}
}

static void keyCallbackGLFW(GLFWwindow* _window, int key, int scancode, int action, int mods) {
	(void)scancode; // suppress unused warning

	bool ctrl = (mods & GLFW_MOD_CONTROL) != 0;
	bool shift = (mods & GLFW_MOD_SHIFT) != 0;
	
	// Solo Keys
	switch (key) {
		case GLFW_KEY_PAUSE:
			if (action == GLFW_PRESS)
				glfwSetWindowShouldClose(_window, true);
			break;
	}

	// Ctrl + Shift + Key
	if (ctrl && shift) {
		switch (key) {
			case GLFW_KEY_KP_ADD: // Numpad +
			case GLFW_KEY_EQUAL:  // Regular +
				if (action == GLFW_PRESS) {
					stopContinuousZooming();
					zoomingInSlow = true;
				} else if (action == GLFW_RELEASE) {
					stopContinuousZooming();
				}
				break;

			case GLFW_KEY_KP_SUBTRACT: // Numpad -
			case GLFW_KEY_MINUS:       // Regular -
				if (action == GLFW_PRESS) {
					stopContinuousZooming();
					zoomingOutSlow = true;
				} else if (action == GLFW_RELEASE) {
					stopContinuousZooming();
				}
				break;
		}
	}

	// Ctrl + Key without shift
	if (ctrl && !shift) {
		switch (key) {
			case GLFW_KEY_KP_ADD: // Numpad +
			case GLFW_KEY_EQUAL:  // Regular +
				if (action == GLFW_PRESS) {
					stopContinuousZooming();
					zoomingIn = true;
				} else if (action == GLFW_RELEASE) {
					stopContinuousZooming();
				}
				break;

			case GLFW_KEY_KP_SUBTRACT: // Numpad -
			case GLFW_KEY_MINUS:       // Regular -
				if (action == GLFW_PRESS) {
					stopContinuousZooming();
					zoomingOut = true;
				} else if (action == GLFW_RELEASE) {
					stopContinuousZooming();
				}
				break;
		}
	}

	// Ctrl + Key
	if (ctrl) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				if (action == GLFW_PRESS)
					glfwSetWindowShouldClose(_window, true);
				break;

			case GLFW_KEY_ENTER:
				if (action == GLFW_PRESS)
					ImGuiEnabled = !ImGuiEnabled;
				break;
		}
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

	ImFont* font = io.Fonts->AddFontFromFileTTF("../res/ImGuiFonts/Roboto-Medium.ttf", 15.0f);
	if (font == nullptr)
		std::cout << "Error: Font for ImGui could not be loaded" << std::endl;
}

static void initUniformVariables() {
	// Coordinate Mapping
	shader.setVec2UInt("windowSize", { windowWidth, windowHeight });
	shader.setDouble("zoomScale", static_cast<double>(zoomScale));
	shader.setVec2Double("center", { centerX, centerY });
	shader.setVec2UInt("tileOffset", { 0u, 0u }); // only needed for tiled screenshot rendering
	
	// RK45
	shader.setUInt("MAX_STEPS", static_cast<unsigned int>(getMaxIterations()));
	shader.setUInt("MAX_SAME_STEPS", maxSameSteps);
	shader.setFloat("MIN_TAU", minStepSize);
	shader.setFloat("atol", std::pow(10.0f, atolExponent));
	shader.setFloat("rtol", std::pow(10.0f, rtolExponent));

	// Double Pendulum
	shader.setFloat("t_end", simulationEndTime);
	shader.setFloat("v1_start", v1Start);
	shader.setFloat("v2_start", v2Start);
	shader.setFloat("g", weightConstant);
	shader.setFloat("l1", length1);
	shader.setFloat("l2", length2);
	shader.setFloat("m1", mass1);
	shader.setFloat("m2", mass2);

	// N Body Problem
	// shader.setFloat("t_end", simulationEndTime);
	// // shader.setFloat("g", gravitationalConstant);
	// shader.setFloatArray("m", masses, N);
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

	// Shader
	//shader = { "../res/vertex_shader.glsl", + "../res/fragment_shader.glsl", false, false }; // Compile and link shader, but keep sources, ...
	shader = Shader("../res/vertex_shader.glsl", + "../res/fragment_shader_double_pendulum.glsl", false, false); // Compile and link shader, but keep sources, ...
	// shader = Shader("../res/vertex_shader.glsl", + "../res/fragment_shader_n_body_problem.glsl", false, false); // Compile and link shader, but keep sources, ...
	// define default values 
	//shader.define(FLOW_COLOR_TYPE, DEFAULT_FLOW_COLOR_TYPE);
	//shader.define(CODE_DIVERGENCE_CRITERION, codeDivergenceCriterion);
	//shader.define(CODE_CALCULATE_NEXT_SEQUENCE_TERM, codeCalculateNextSequenceTerm);
	shader.compileVertexShader();
	shader.compileFragmentShader();
	shader.link();
	shader.use(); // Needs to be called before setting uniform variables

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
	// unsigned int vertexArray;
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

	initUniformVariables();

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		using timePoint = decltype(std::chrono::high_resolution_clock::now());
		timePoint startTime =  std::chrono::high_resolution_clock::now();

		static bool showImGuiWindow = true;
		if (ImGuiEnabled) {
			ImGuiFrame(showImGuiWindow);
		}

		// use program (should be called in the loop, since other parts could use other programs in the meantime, according to ChatGPT)
		shader.use();
	
		if (ImGuiEnabled)
			ImGui::Render();

		// clear screen
		glClearColor(0.0f, 0.05f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// draw
		glBindVertexArray(vertexArray);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

		if (ImGuiEnabled)
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// check and call events ans swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		timePoint endTime = std::chrono::high_resolution_clock::now();
		auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		// Zooming
		if (zoomingIn) {
			zoom(1.0 / std::pow(ZOOM_PER_SECOND, static_cast<double>(delta.count())/1000.0));
		} else if (zoomingOut) {
			zoom(std::pow(ZOOM_PER_SECOND, static_cast<double>(delta.count())/1000.0));
		} else if (zoomingInSlow) {
			zoom(1.0 / std::pow(SLOW_ZOOM_PER_SECOND, static_cast<double>(delta.count())/1000.0));
		} else if (zoomingOutSlow) {
			zoom(std::pow(SLOW_ZOOM_PER_SECOND, static_cast<double>(delta.count())/1000.0));
		}

		if (ImGuiEnabled) {
			lastFrameDeltas[lastFrameArrayIndex++] = (1000.0f / static_cast<float>(delta.count()));
			if (lastFrameArrayIndex == lastFrameDeltas.size())
				lastFrameArrayIndex = 0;
		}

		if (!showImGuiWindow) {
			ImGuiEnabled = false;
			showImGuiWindow = true;
		}
	}

	// delete all resources (not necessary)
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &elementBuffer);
	shader.clean();
	shader.deleteProgram();

	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}
