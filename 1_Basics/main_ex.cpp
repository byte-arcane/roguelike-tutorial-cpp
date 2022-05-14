#include <framework.h>
#include <utility.h>
#include <GL/glew.h>
#include <imgui.h>

using namespace glm;
using namespace std;

class FrameworkAppBasics : public cgf::FrameworkApp
{
	unsigned int shaderProgram=0, VBO=0, VAO=0;
	int numVertices = 0;

	bool isWireframe = false;

	// put the shader programs here as raw text -- later on we'll be putting them in files
	const char* vertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		void main()
		{
		   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
		})";
	const char* fragmentShaderSource = R"(
		#version 330 core
		out vec4 FragColor;
		void main()
		{
		   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
		})";

	// Put here any initialisation code. Happens once, before the main loop and after initialisation of GLFW/GLEW/ImGui
	void onInit() override 
	{
		// build and compile our shader program
		shaderProgram = cgf::buildShader(vertexShaderSource, fragmentShaderSource);

		// create a number of 3D vertices that form a triangle, specified in a counter-clockwise manner 
		vector<vec3> vertices =
		{
			vec3(- 0.5f, -0.5f, 0.0f), // left
			vec3(0.5f, -0.5f, 0.0f), // right
			vec3(0.0f,  0.5f, 0.0f)  // top
		};
		// provide the address of the first float element in the vertices array, and the TOTAL size in bytes
		VBO = cgf::buildVBO(&vertices[0].x, vertices.size() * sizeof(vec3));
		// create the VAO which stores the 
		VAO = cgf::buildVAO(VBO, sizeof(vec3));
		numVertices = int(vertices.size());
	}

	// Put here any termination code. Happens once, before termination of GLFW/GLEW/ImGui
	void onTerminate() override 
	{
		// OpenGL-related
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteProgram(shaderProgram);
	}

	// Put here any rendering code. Called every frame
	void onRender() override 
	{
		// the frame starts with a clean scene
		glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// activate the shader
		glUseProgram(shaderProgram);

		// Bind, draw and unbind the geometry
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
		glBindVertexArray(0);

		// deactivate the shader
		glUseProgram(0);
	}

	// Put here any GUI related code. Called after render
	void onGui() override 
	{
		ImGui::LabelText("TextLabel","Hello world!");

		// Check if the isWireframe variable changes. By default it's false
		bool isWireframeChanged = ImGui::Checkbox("Wireframe", &isWireframe);
		if (isWireframeChanged)
		{
			if(isWireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
};

int main(int argc, char** argv)
{
	FrameworkAppBasics app;
	return app.run();
}