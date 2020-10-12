#pragma once

#include <vector>

#include "Vertex.h"

class VertexArray
{
	public:
        /**
		 * parameters:
		 * 		vertices: The vertex with all of its data.
		 * 		indices: Used to index into vertices allowing triangles to share vertices.
		 * 			Set to nullptr if not used.
        */
		VertexArray(const std::vector<Vertex> &vertices,
				GLenum verticesUsage,
				const std::vector<unsigned int>* indices,
				GLenum indicesUsage);
		~VertexArray();
		unsigned int getId() const;
		void bind() const;
		void updateElementBuffer(const std::vector<unsigned int> &indices, GLenum usage);

	private:
		unsigned int id;
		unsigned int vertexBufferId;
		unsigned int elementBufferId;
};
