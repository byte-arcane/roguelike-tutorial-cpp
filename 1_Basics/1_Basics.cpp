#include <framework.h>
#include <utility.h>
#include <GL/glew.h>
#include <imgui.h>


#include <glm/glm.hpp>

using namespace glm;
using namespace std;

class FrameworkAppBasics : public cgf::FrameworkApp
{
	unsigned int shaderProgram=0, VBO=0, VAO=0;
	int numVertices = 0;

#ifdef SOLUTIONS
	GLuint VBO_cw = 0, VAO_cw = 0;
	int numVertices_cw = 0;
#endif

	bool isWireframe = false;

	// Put here any initialisation code. Happens once, before the main loop and after initialisation of GLFW/GLEW/ImGui
	void onInit() override 
	{
		// build and compile our shader program
		auto vs = cgf::readTextFile(cgf::mediaSearch("shaders/1_Basics.vert"));
		auto fs = cgf::readTextFile(cgf::mediaSearch("shaders/1_Basics.frag"));
		shaderProgram = cgf::buildShader(vs.c_str(), fs.c_str());

		// create a number of 3D vertices that form a triangle, specified in a counter-clockwise manner 
		vector<vec3> vertices =
		{
			vec3(- 0.5f, -0.5f, 0.0f), // left
			vec3(0.5f, -0.5f, 0.0f), // right
			vec3(0.0f,  0.5f, 0.0f)  // top
		};
		// provide the address of the first float element in the vertices array, and the TOTAL size in bytes
		VBO = cgf::buildVBO(&vertices[0].x, int(vertices.size() * sizeof(vec3)));
		// create the VAO which stores the 
		VAO = cgf::buildVAO(VBO, sizeof(vec3));
		numVertices = int(vertices.size());

#ifdef SOLUTIONS
		const float CELL_SIZE = 0.1f;
		const int CELL_NUM = 10;
		const float GRID_START = -0.5f;
		std::vector<vec3> vgrid;
		for(int y=0;y< CELL_NUM;++y)
			for (int x = 0; x < CELL_NUM; ++x)
			{
				// specify vertices
				glm::vec3 v00 = vec3(GRID_START + x*CELL_SIZE, GRID_START + y*CELL_SIZE, 0.0f);
				glm::vec3 v10 = vec3(GRID_START + (x+1) * CELL_SIZE, GRID_START + y * CELL_SIZE, 0.0f);
				glm::vec3 v01 = vec3(GRID_START + x * CELL_SIZE, GRID_START + (y+1) * CELL_SIZE, 0.0f);
				glm::vec3 v11 = vec3(GRID_START + (x+1) * CELL_SIZE, GRID_START + (y + 1) * CELL_SIZE, 0.0f);

				// tri1
				vgrid.push_back(v00);
				vgrid.push_back(v10);
				vgrid.push_back(v11);

				// tri2
				vgrid.push_back(v11);
				vgrid.push_back(v01);
				vgrid.push_back(v00);
			}

		numVertices_cw = int(vgrid.size());
		// provide the address of the first float element in the vertices array, and the TOTAL size in bytes
		VBO_cw = cgf::buildVBO(&vgrid[0].x, int(vgrid.size() * sizeof(vec3)));
		// create the VAO which stores the 
		VAO_cw = cgf::buildVAO(VBO_cw, sizeof(vec3));
#endif
	}

	// Put here any termination code. Happens once, before termination of GLFW/GLEW/ImGui
	void onTerminate() override 
	{
		// OpenGL-related
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteProgram(shaderProgram);

#ifdef SOLUTIONS
		glDeleteVertexArrays(1, &VAO_cw);
		glDeleteBuffers(1, &VBO_cw);
#endif
	}

	// Put here any rendering code. Called every frame
	void onRender() override 
	{
		// the frame starts with a clean scene
		glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// activate the shader
		glUseProgram(shaderProgram);

#ifndef SOLUTIONS
		// Bind, draw and unbind the geometry
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
		glBindVertexArray(0);
#endif

#ifdef SOLUTIONS
		glBindVertexArray(VAO_cw);
		glDrawArrays(GL_TRIANGLES, 0, numVertices_cw);
		glBindVertexArray(0);
#endif

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