#include <glad/glad.h>
#include <glm/glm.hpp>

#include "EdgeBuffer.h"

namespace{
/**
 *	Sorts a, b, and c in ascending order.
 */
void sort(unsigned int& a, unsigned int& b, unsigned int& c)
{
	if (a > b)
	{
		std::swap(a, b);
	}
	if (b > c)
	{
		std::swap(b, c);
	}
	if (a > b)
	{
		std::swap(a, b);
	}
}
}


EdgeBuffer::EdgeBuffer(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices):
	vertices(vertices), indices(indices)
{
	createEdgeBuffer();
	edgeVertexArray = std::make_unique<VertexArray>(
			vertices,
			GL_STATIC_DRAW,
			nullptr,
			GL_DYNAMIC_DRAW);
}

/*
 * Creates the EdgeBuffer with default values. The edge buffer will then
 * need to be updated every frame.
 *
 * Assumes that there is only one mesh for this model.
 */
void EdgeBuffer::createEdgeBuffer()
{
	edgeBuffer.resize(vertices.size());	
	std::fill(edgeBuffer.begin(), edgeBuffer.end(), std::vector<EdgeInfo>());

	size_t numTriangles = indices.size() / 3;

	for (unsigned int i = 0; i < numTriangles; i++)
	{
		unsigned int index0 = indices[i * 3 + 0];
		unsigned int index1 = indices[i * 3 + 1];
		unsigned int index2 = indices[i * 3 + 2];
		
		sort(index0, index1, index2);
		insertEdge(index0, index1);
		insertEdge(index0, index2);
		insertEdge(index1, index2);
	}
}

/**
 * Inserts an edge between index0 and index1. Make sure the indices
 * are sorted for optimal performance.
 */
void EdgeBuffer::insertEdge(unsigned int index0, unsigned int index1)
{
	EdgeInfo edge;
	edge.vertex = index1;
	edge.artist = false;
	edge.front = false;
	edge.back = false;
	edge.absoluteFront = false;
	edge.absoluteBack = false;

	bool found = false;
	for (unsigned int j = 0; j < edgeBuffer[index0].size() && !found; j++)
	{
		found = edgeBuffer[index0][j].vertex == index1;	
	}

	if (!found)	// if not found or if vector is empty for index0.
	{
		edgeBuffer[index0].push_back(edge);
	}
}

/*
 * Draws the silhouette edges stored in this edge buffer.
 * Remember to bind the shader before calling this function.
 */
void EdgeBuffer::draw() const
{
	edgeVertexArray->bind();
	glLineWidth(4.0f);
	glDrawElements(GL_LINES, edgeIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

/**
 *	Iterate through every triangle of the mesh and store whether
 *	the edges of the triangle are front facing or back facing
 *	into the edge buffer.
 */
void EdgeBuffer::update(const glm::vec3& toCamera, const glm::mat4& model)
{
	size_t numTriangles = indices.size() / 3;
	glm::vec3 toCameraNorm = glm::normalize(toCamera);

	for (unsigned int i = 0; i < numTriangles; i++)
	{
		unsigned int index0 = indices[i * 3 + 0];
		unsigned int index1 = indices[i * 3 + 1];
		unsigned int index2 = indices[i * 3 + 2];

		const Vertex& vertex0 = vertices[index0];
		const Vertex& vertex1 = vertices[index1];
		const Vertex& vertex2 = vertices[index2];

		// Compute cross product to get normal. Vertices of the triangle
		// are assumed to be in a counter-clock wise winding order.
		glm::vec3 a = vertex1.position - vertex0.position;
		glm::vec3 b = vertex2.position - vertex1.position;
		glm::vec3 normal = glm::normalize(glm::cross(a, b));

		// Take the average of the 3 normals.
		//glm::vec3 normal = vertex0.normal + vertex1.normal + vertex2.normal;
		normal = glm::normalize(model * glm::vec4(normal, 0.0f));
		float dot = glm::dot(normal, toCameraNorm);

		// Sort after computing the normal to preserve winding order.
		sort(index0, index1, index2);

		if (dot >= 0.0f)
		{
			setFrontEdge(index0, index1);
			setFrontEdge(index0, index2);
			setFrontEdge(index1, index2);
		}
		else
		{
			setBackEdge(index0, index1);
			setBackEdge(index0, index2);
			setBackEdge(index1, index2);
		}
	}

	// Store all silhouette edges.
	edgeIndices.clear();
	for (unsigned int i = 0; i < edgeBuffer.size(); i++)
	{
		for (const auto& edge : edgeBuffer[i])
		{
			if (edge.front && edge.back)
			{
				edgeIndices.push_back(i);
				edgeIndices.push_back(edge.vertex);
			}
		}
	}
	edgeVertexArray->updateElementBuffer(edgeIndices, GL_DYNAMIC_DRAW);
}

/**
 * Performs a XOR on the front edge (index0, inde1).
 */
void EdgeBuffer::setFrontEdge(unsigned int index0, unsigned int index1)
{
	for (EdgeInfo& edge : edgeBuffer[index0])
	{
		if (edge.vertex == index1)
		{
			// Logically equivalent to edge.front XOR true.
			edge.front = !edge.front;
			break;
		}
	}
}

/**
 * Performs a XOR on the back edge (index0, inde1).
 */
void EdgeBuffer::setBackEdge(unsigned int index0, unsigned int index1)
{
	for (EdgeInfo& edge : edgeBuffer[index0])
	{
		if (edge.vertex == index1)
		{
			// Logically equivalent to edge.back XOR true.
			edge.back = !edge.back;
			break;
		}
	}
}

/**
 *	Reset the value of the front and back edges. Should be called
 *	after draw().
 */
void EdgeBuffer::reset()
{
	for (auto& vertex : edgeBuffer)
	{
		for (EdgeInfo& edge : vertex)
		{
			edge.front = false;
			edge.back = false;
		}
	}

}

/**
 * Prints the contents of the edge buffer.
 *
 * Code taken from TA Alex Garcia.
 */
void EdgeBuffer::print()
{
	printf("Printing EdgeBuffer: \n");
	for (uint32_t i = 0; i < edgeBuffer.size(); i++)
	{
		printf("%d\t", i);
		for (uint32_t j = 0; j < edgeBuffer[i].size(); j++)
		{
			printf("%d-%d-%d ",
					edgeBuffer[i][j].vertex,
					edgeBuffer[i][j].front,edgeBuffer[i][j].back);
		}
		printf("\n");
	}
	printf("\n\n");
}
