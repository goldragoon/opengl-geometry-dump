#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
using namespace glm;

// Automatically link in the GLUT and GLEW libraries if compiling on MSVC++
#ifdef _MSC_VER
#pragma comment(lib, "glew32")
#pragma comment(lib, "freeglut")
#endif

#include <iostream>
#include <vector>
//#include <unordered_map>
using namespace std;

//-------------------------------- OpenMesh-Related ------------------------------------

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

using namespace OpenMesh;
using namespace OpenMesh::IO;

#define OpenMeshVectorType OpenMesh::Vec3f
struct MyTraits : public DefaultTraits {	typedef OpenMeshVectorType Point; };
typedef TriMesh_ArrayKernelT<MyTraits>  MyMesh;

//-------------------------------------------------------------------------------------

#include "vertex_geometry_shader.h"
std::vector<float> vertex_positions; // same data type as OpenMeshVectorType's component, storing vertex position as [..., v_i.x, v_i.y, v_i.z, ... ]
std::vector<int> face_indices;

void readMesh(std::string file)
{
	MyMesh mesh;
	OpenMesh::IO::read_mesh(mesh, file);
	
	for (auto face : mesh.faces())
	{
		for (auto vertex : mesh.fv_range(face))
		{
			auto point = mesh.point(vertex);
			vertex_positions.push_back(point[0]);
			vertex_positions.push_back(point[1]);
			vertex_positions.push_back(point[2]);
			face_indices.push_back(vertex.idx());
		}
	}
}
void writeMesh(MyMesh& mesh, std::string file) { OpenMesh::IO::write_mesh(mesh, file); }

int main(int argc, char **argv)
{
	bool face_division = false;
	// Read input data.
	readMesh("data/bunny.obj");
	printf("Input mesh specifications : (#V : %u), (#F : %u)\n",  vertex_positions.size() / 3, face_indices.size() / 3);

	// Initialize OpenGL context.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(10, 10);
	glutInitWindowPosition(0, 0);

	GLint win_id = glutCreateWindow("Example for extracting geometry from graphics pipeline");

	if (GLEW_OK != glewInit())
	{
		cout << "GLEW initialization error" << endl;
		return 0;
	}

	int GL_major_version = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &GL_major_version);

	int GL_minor_version = 0;
	glGetIntegerv(GL_MINOR_VERSION, &GL_minor_version);

	if (GL_major_version < 4)
	{
		cout << "GPU does not support OpenGL 4.3 or higher" << endl;
		return 0;
	}
	else if (GL_major_version == 4)
	{
		if (GL_minor_version < 3)
		{
			cout << "GPU does not support OpenGL 4.3 or higher" << endl;
			return 0;
		}
	}

	int num_varying_comp_per_vertex = 4; // 3 + 1
	std::vector<const char*> varying_names;
	varying_names.push_back("TF_VPOS"); // 3 components
	varying_names.push_back("TF_VID"); // 1 components

	// Initialize shader.
	vertex_geometry_shader g0_mc_shader;
	if (false == g0_mc_shader.init("shader/tf.vs", "shader/tf-triangles.gs", varying_names))
	{
		cout << "Couldn't load shaders" << endl;
		return 0;
	}
	g0_mc_shader.use_program();

	// Allocate and transfer mesh geometry information to GPU.
	vector<float> point_vertex_data;
	point_vertex_data = vertex_positions;

	const GLuint components_per_position = 3;
	const GLuint components_per_vertex = components_per_position;

	GLuint point_buffer;
	glGenBuffers(1, &point_buffer);
	const GLuint num_vertices = static_cast<GLuint>(point_vertex_data.size()) / components_per_vertex;
	glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
	glBufferData(GL_ARRAY_BUFFER, point_vertex_data.size() * sizeof(GLfloat), &point_vertex_data[0], GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(g0_mc_shader.get_program(), "position"));
	glVertexAttribPointer(glGetAttribLocation(g0_mc_shader.get_program(), "position"),
		components_per_position,
		GL_FLOAT,
		GL_FALSE,
		components_per_vertex * sizeof(GLfloat),
		0);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_indices.size() * sizeof(int), face_indices.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);

	size_t max_triangles_per_geometry_shader = 1;
	size_t num_vertices_per_primitive = 3;

	// Allocate enough for the maximum number of generated triangles in Geometry Shader (or final stage of transformed feedback)
	GLuint tbo;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * num_vertices * max_triangles_per_geometry_shader * num_vertices_per_primitive * num_varying_comp_per_vertex, nullptr, GL_STATIC_READ);

	GLuint query;
	glGenQueries(1, &query);

	// Perform feedback transform
	glEnable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_TRIANGLES);
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glDisable(GL_RASTERIZER_DISCARD);
	glFlush();

	// Get number of primitives while feedback transform occurs.
	GLuint primitives;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);

	// Read back actual number of triangles (in case it's less than two triangles)
	printf("[Start] Extract mesh from transform feedback buffer\n");
	vector<GLfloat> feedback(primitives * num_vertices_per_primitive * num_varying_comp_per_vertex);
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(GLfloat)* feedback.size(), &feedback[0]);
	printf("[End] Extract mesh from transform feedback buffer\n");

	glDeleteQueries(1, &query);
	glDeleteBuffers(1, &tbo);

	printf("[Start] Write mesh to disk\n");
	MyMesh output_msh;
	for (size_t i = 0; i < primitives; i++)
	{
		size_t feedback_primitive_index = num_varying_comp_per_vertex * num_vertices_per_primitive * i;
		vector<MyMesh::VertexHandle> primitive_vertices_indices;

		for (size_t j = 0; j < num_vertices_per_primitive; j++) {
			unsigned int comp_index = j * num_varying_comp_per_vertex;
			primitive_vertices_indices.push_back(output_msh.add_vertex(MyTraits::Point(
				feedback[feedback_primitive_index + comp_index + 0],
				feedback[feedback_primitive_index + comp_index + 1],
				feedback[feedback_primitive_index + comp_index + 2])));
			//vhs.push_back(output_msh.add_vertex(MyTraits::Point(feedback[feedback_index + 3], feedback[feedback_index + 4], feedback[feedback_index + 5])));
			//vhs.push_back(output_msh.add_vertex(MyTraits::Point(feedback[feedback_index + 6], feedback[feedback_index + 7], feedback[feedback_index + 8])));
		}
		output_msh.add_face(primitive_vertices_indices);
	}
	OpenMesh::IO::write_mesh(output_msh, "output.obj");
	printf("[End] Write mesh to disk\n");
	return 0;
}