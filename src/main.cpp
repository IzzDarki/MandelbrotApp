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
#include "model/model_double_pendulum.h"
#include "model/model_mandelbrot.h"

static GLFWwindow* window;
//static Shader shader;

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
static bool lockZoomFocusToCenter = false;
static constexpr long double ZOOM_STEP = 1.1L; // single zoom step (multiplicative)
static constexpr long double ZOOM_STEP_SMALL = 1.01L;
static constexpr double ZOOM_PER_SECOND = 3.0; // continuous zoom (multiplicative)
static constexpr double SLOW_ZOOM_PER_SECOND = 1.3;

static std::array<float, 10> lastFrameDeltas;
static std::size_t lastFrameArrayIndex = 0;
static bool autoMaxIterations = false;

static std::unique_ptr<Model> model = std::make_unique<DoublePendulumModel>();
static std::unique_ptr<Model> screenshotModel; // Initialized in main (always kept in a state, where at least the vertex shader is compiled)

// N-Body Problem Parameters
// constexpr const uint N = 3u;
// constexpr const uint D = 3u;
// static float simulationEndTime = 0.0f;
// static float masses[N] = { 1.0f, 0.9f, 1.2f };

static bool ImGuiEnabled = true;

// * HELPER FUNCTIONS

// static int getMaxIterations() {
// 	//int zoomCount = -std::log(zoomScale / 3.5) / std::log(ZOOM_STEP); // how often you have zoomed in

// 	if (autoMaxIterations) {
// 		maxIterations = static_cast<int>(400 + 100L * -std::log10(zoomScale));
// 		if (maxIterations < 200)
// 			maxIterations = 200;
// 		else if (maxIterations > 4000)
// 			maxIterations = 4000;
// 	}
// 	return maxIterations;
// }


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

static void applyGlobalUniformVariables(Model& model); // forward declaration

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
	long double zoomFocusX = static_cast<long double>(windowWidth) / 2.0;
	long double zoomFocusY = static_cast<long double>(windowHeight) / 2.0;

	if (!lockZoomFocusToCenter) {
		// Get mouse pos
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		mouseY = windowHeight - mouseY;

		// Set focus point to mouse pos
		zoomFocusX = static_cast<long double>(mouseX);
		zoomFocusY = static_cast<long double>(mouseY);
	}
	
	auto windowSize = getWindowSize();
	centerX += ((1.0L - factor) * zoomScale / windowSize) * (zoomFocusX + 0.5L - windowWidth / 2.0L);
	centerY += ((1.0L - factor) * zoomScale / windowSize) * (zoomFocusY + 0.5L - windowHeight / 2.0);
	zoomScale *= factor;

	model->shader.setDouble("zoomScale", static_cast<double>(zoomScale));
	model->shader.setVec2Double("center", { centerX, centerY });
}

static void jumpToView(const SavedView& savedView) {
	zoomScale = savedView.getZoomScale();
	centerX = savedView.getCenter().first;
	centerY = savedView.getCenter().second;

	model->shader.setDouble("zoomScale", static_cast<double>(zoomScale));
	model->shader.setVec2Double("center", { centerX, centerY });
}

static void ImGuiFrame(bool& showImGuiWindow) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("Options", &showImGuiWindow, ImGuiWindowFlags_NoCollapse)) {	// Create a window and append into it.
		ImGui::SetWindowSize({0,0}, ImGuiCond_FirstUseEver); // set window to fit contents when first creating it (ImGui saves position between sessions)

		if (lockZoomFocusToCenter) {
			ImGui::Text("Zooming locked to center of window");
		}
		if (ImGui::BeginTabBar("#idTabBar"))
		{
			if (ImGui::BeginTabItem("Info"))
			{
				//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::Text("%.1f fps", static_cast<double>(calcFPSAverage()));
				
				model->imGuiFrame();

				

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
				if (ImGui::CollapsingHeader("Info")) {
					ImGui::Text("Zoom: %.1Le", zoomScale);
					auto [real, imag] = getNumberAtCursor();
					ImGui::Text("Cursor: %.10Lf + %.10Lf i", real, imag);
					ImGui::Text("Center: %.10Lf + %.10LF i", centerX, centerY);
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Saved views")) {
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

				ImGui::InputText("Filename", screenshotFilename, sizeof(screenshotFilename));
				ImGui::InputInt("Width", &captureWidth);
				ImGui::InputInt("Height", &captureHeight);
				ImGui::InputInt("Max Tile Size", &maxTileSize);

				ImGui::Separator();

				// Replace the screenshot model if the subclass type changed in the live model
				if (typeid(*model) != typeid(*screenshotModel)) {
					std::unique_ptr<Model> oldScreenshotModel = std::move(screenshotModel);
					screenshotModel = model->clone();
					screenshotModel->makeScreenshotModel(*oldScreenshotModel);
					screenshotModel->shader.compileVertexShader(); // Needed, because recompile does not recompile the vertex shader
					
					// Assertion as sanity-check (maybe remove this at some point)
					if (typeid(*model) != typeid(*screenshotModel)) {
						std::cerr << "This should never not happen " << __FILE__ << " line " << __LINE__
							<< "typeid(*model) is " << typeid(*model).name()
							<< ", typeid(*screenshotModel) is " << typeid(*screenshotModel).name() << std::endl;
					}
					std::cout << "Switched screenshot model" << std::endl;
				}

				// Show ImGui UI for screenshot
				screenshotModel->imGuiScreenshotFrame();

				ImGui::Separator();

				if (ImGui::Button("Take Screenshot")) {
					// Bring screenshot model up to date (e.g. changed simulationEndTime)
					screenshotModel->updateWithLiveModel(*model); // probably not necessary if screenshotModel was cloned in the if statement above, but can't hurt
					screenshotModel->shader.recompile(); // only needed when updateWithLiveModel changes any defines, but recompiling should not take too much effort
														 // recompile calls Shader::use()

					// Apply the screenshots uniform variables
					applyGlobalUniformVariables(*screenshotModel);
					screenshotModel->applyUniformVariables();

					// Take the screenshot
					takeScreenshot(
						screenshotFilename,
						static_cast<size_t>(std::max(captureWidth, 0)),
						static_cast<size_t>(std::max(captureHeight, 0)),
						*screenshotModel,
						vertexArray,
						static_cast<size_t>(maxTileSize)
					);
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Help"))
			{
				ImGui::Text("Press Ctrl + Enter to toggle GUI");
				ImGui::Text("Press Ctrl + Esc or Pause to exit the app");
				ImGui::Text("Press Ctrl + Plus/Minus to zoom in/out (numpad works too)");
				ImGui::Text("Use Mouse Wheel to zoom in/out");
				// TODO Write all controls here
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

	model->shader.setVec2UInt("windowSize", { windowWidth, windowHeight });
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
			if (action == GLFW_PRESS) {
				glfwSetWindowShouldClose(_window, true);
			}
			break;
		case GLFW_KEY_C:
			if (action == GLFW_PRESS) {
				
			}
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
			case GLFW_KEY_C:
				if (action == GLFW_PRESS) {
					lockZoomFocusToCenter = !lockZoomFocusToCenter;
				}
				break;

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

static void mouseButtonCallbackGLFW(GLFWwindow* _window, int button, int action, int mods) {
	(void)_window;

	bool ctrl = (mods & GLFW_MOD_CONTROL) != 0;
	// bool shift = (mods & GLFW_MOD_SHIFT) != 0;

    if (ctrl && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		const auto num = getNumberAtCursor();
        centerX = num.first;
		centerY = num.second;
		model->shader.setVec2Double("center", { centerX, centerY });
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
	glfwSetMouseButtonCallback(window, mouseButtonCallbackGLFW);

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

static void applyGlobalUniformVariables(Model& usedModel) {
	// Coordinate Mapping
	usedModel.shader.setVec2UInt("windowSize", { windowWidth, windowHeight });
	usedModel.shader.setDouble("zoomScale", static_cast<double>(zoomScale));
	usedModel.shader.setVec2Double("center", { centerX, centerY });
	usedModel.shader.setVec2UInt("tileOffset", { 0u, 0u }); // only needed for tiled screenshot rendering

	// N Body Problem
	// shader.setFloat("t_end", simulationEndTime);
	// // shader.setFloat("g", gravitationalConstant);
	// shader.setFloatArray("m", masses, N);
}


// * MAIN FUNCTION

int main()
{
	// Initialize screenshotModel
	screenshotModel = model->clone();
	screenshotModel->makeScreenshotModel(); // e. g. set lower rk45 tolerance to achieve better quality screenshots than the live view

	SavedView::initFromFile();

	if (!initGLFW())
		return -1;
	if (!initGlad())
		return -1;
	initImGui();

	screenshotModel->shader.compileVertexShader(); // screenshotModel must always have the vertex shader compiled
	model->shader.compileAndLink();
	model->shader.use(); // Needs to be called before setting uniform variables

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

	applyGlobalUniformVariables(*model);
	model->applyUniformVariables();

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		using timePoint = decltype(std::chrono::high_resolution_clock::now());
		timePoint startTime =  std::chrono::high_resolution_clock::now();

		static bool showImGuiWindow = true;
		if (ImGuiEnabled) {
			ImGuiFrame(showImGuiWindow);
		}

		// use program (should be called in the loop, since other parts could use other programs in the meantime, according to ChatGPT)
		model->shader.use();
	
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
	model->shader.destroy(); // destroy before destroying the OpenGLs context

	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}
