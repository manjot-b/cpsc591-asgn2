#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>

#include "Shader.h"
#include "Mesh.h"
#include "EdgeBuffer.h"

class Model
{
	public:
		Model(const std::string &objPath, const Shader& shader);
		~Model();
		void draw() const;
		void update();
		void rotate(const glm::vec3 &rotate);
		void scale(float scale);
		void updateEdgeBuffer(const glm::vec3& cameraPosition);
		void drawEdgeBuffer() const;
		void resetEdgeBuffer();

	private:

		const Shader& shader;
		std::vector<std::unique_ptr<Mesh>> meshes;
		std::unique_ptr<EdgeBuffer> edgeBuffer;

		BoundingBox boundingBox;
		glm::mat4 modelMatrix;
		glm::vec3 m_rotate;			// how much to rotate along each axis
		float m_scale;				// scale to apply to model
		glm::vec3 m_translation;	// translation vector

		void extractDataFromNode(const aiScene* scene, const aiNode* node);
		void scaleToViewport();
};
