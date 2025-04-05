#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class PlaneMesh {
	std::vector<float> verts;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	GLuint VAO, VBO, NBO, EBO;
	GLuint shaderID;
	GLuint waterTextureID, dispTextureID; // Optional

	float min, max;
	int numVerts, numIndices;
	glm::vec4 modelColor;

	void planeMeshQuads(float min, float max, float stepsize) {
		float y = 0;
		for (float x = min; x <= max; x += stepsize) {
			for (float z = min; z <= max; z += stepsize) {
				verts.push_back(x);
				verts.push_back(y);
				verts.push_back(z);
				normals.push_back(0);
				normals.push_back(1);
				normals.push_back(0);
			}
		}

		int nCols = (max - min) / stepsize + 1;
		for (int i = 0; i < nCols - 1; ++i) {
			for (int j = 0; j < nCols - 1; ++j) {
				indices.push_back(i * nCols + j);
				indices.push_back(i * nCols + j + 1);
				indices.push_back((i + 1) * nCols + j + 1);
				indices.push_back((i + 1) * nCols + j);
			}
		}
	}

public:
	PlaneMesh(float min, float max, float stepsize, GLuint shaderProgram, GLuint waterTex = 0, GLuint dispTex = 0)
		: shaderID(shaderProgram), waterTextureID(waterTex), dispTextureID(dispTex)
	{
		this->min = min;
		this->max = max;
		modelColor = glm::vec4(0, 1.0f, 1.0f, 1.0f);

		planeMeshQuads(min, max, stepsize);

		numVerts = verts.size() / 3;
		numIndices = indices.size();

		printf("\n\nverts: %d || indices: %d\n\n", numVerts, numIndices);

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &NBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		// Vertex positions
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), &verts[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0);

		// Normals
		glBindBuffer(GL_ARRAY_BUFFER, NBO);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(1);

		// Indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	void draw(glm::vec3 lightPos, glm::mat4 V, glm::mat4 P) {
		glUseProgram(shaderID);

		glm::vec3 eye = glm::vec3(glm::inverse(V)[3]);
		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		glm::mat4 MVP = P * V * ModelMatrix;

		// Upload uniforms
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(ModelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform3fv(glGetUniformLocation(shaderID, "lightPos"), 1, glm::value_ptr(lightPos));
		glUniform3fv(glGetUniformLocation(shaderID, "eyePos"), 1, glm::value_ptr(eye));

		glUniform1f(glGetUniformLocation(shaderID, "outerTess"), 16.0f);
		glUniform1f(glGetUniformLocation(shaderID, "innerTess"), 8.0f);

		float t = glfwGetTime();
		glUniform1f(glGetUniformLocation(shaderID, "time"), t);

		// Bind textures
		if (waterTextureID != 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, waterTextureID);
			glUniform1i(glGetUniformLocation(shaderID, "waterTexture"), 0);
		}
		if (dispTextureID != 0) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, dispTextureID);
			glUniform1i(glGetUniformLocation(shaderID, "displacementTexture"), 1);
		}

		
		// Draw
		glBindVertexArray(VAO);

		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawElements(GL_PATCHES, numIndices, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}
};
