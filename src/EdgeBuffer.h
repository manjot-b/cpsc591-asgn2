# pragma once

#include <vector>
#include <memory>

#include "Vertex.h"
#include "VertexArray.h"

class EdgeBuffer
{
	public:
		EdgeBuffer(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);
		void update(const glm::vec3& cameraPosition, const glm::mat4& model);
		void draw() const;
		void reset();
		void print();

	private:
		struct EdgeInfo
		{
			unsigned int vertex;	// the other vertex index the edge is connected to.
			bool artist;			// Artist edges are always drawn.
			bool front;				// Set based on angle between normal and view vectors.
			bool back;				// Set based on angle between normal and view vectors.
			bool absoluteFront;
			bool absoluteBack;
		};
		const std::vector<Vertex> &vertices;
		const std::vector<unsigned int> &indices;
		std::vector<std::vector<EdgeInfo>> edgeBuffer;
		std::vector<unsigned int> edgeIndices;
		std::unique_ptr<VertexArray> edgeVertexArray;

		void createEdgeBuffer();
		void insertEdge(unsigned int index0, unsigned int index1);
		void setFrontEdge(unsigned int index0, unsigned int index1);
		void setBackEdge(unsigned int index0, unsigned int index1);
};
