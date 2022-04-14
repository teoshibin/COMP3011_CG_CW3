
#include "shapes.h"

using namespace std;

vector<float> getRectangle()
{
	return vector<float> {

		-1.f, 1.f, 0.f, 0, 1,	//tl
		1.f, 1.f, 0.f, 1, 1,	//tr
		1.f, -1.f, 0.f, 1, 0,	//br

		-1.f, 1.f, 0.f, 0, 1,	//tl
		1.f, -1.f, 0.f, 1, 0,	//br
		-1.f, -1.f, 0.f, 0, 0	//bl
	};
}

vector<float> getSkyboxCube()
{
	return vector<float>{
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
}

std::vector<float> getCircle(int num_segments, float radius)
{
	int n_verts = num_segments * 3; // 3 vertex per segment
	int n_points = n_verts * 3; // 3 position values per vertex
	int n_colors = n_verts * 3; // rgb colors per vertex
	int n_texUV = n_verts * 2; // u,v texture coordinates per vertex

	int element_size = n_points; // add new elements or attribute size here if needed

	std::vector<float> circle;
	
	float angle_offset = 360.f / (float)num_segments;
	float current_angle = 0.f;

	for (int i = 0; i < num_segments; i++)
	{

		// circle origin
		circle.push_back(0.f);
		circle.push_back(0.f);
		circle.push_back(0.f);

		// circle ring vertex 1
		circle.push_back((float)sin(DEG2RAD(current_angle)) * radius);
		circle.push_back((float)cos(DEG2RAD(current_angle)) * radius);
		circle.push_back(0.f);

		// increment angle
		current_angle += angle_offset;

		// circle ring vertex 2
		circle.push_back((float)sin(DEG2RAD(current_angle)) * radius);
		circle.push_back((float)cos(DEG2RAD(current_angle)) * radius);
		circle.push_back(0.f);
	}

	return circle;
}
