//
//#include "shapes.h"
//
//float* getArrayCircle(int num_segments, float radius, int* arraySize)
//{
//	int n_verts = num_segments * 3; // 3 vertex per segment
//	int n_points = n_verts * 3; // 3 position values per vertex
//	int n_colors = n_verts * 3; // rgb colors per vertex
//	int n_texUV = n_verts * 2; // u,v texture coordinates per vertex
//
//	*arraySize = sizeof(float) * (n_points);
//
//	float* verts = (float*)malloc(*arraySize);
//
//	float angle_offset = 360.f / (float)num_segments;
//	float current_angle = 0.f;
//
//	if (verts != NULL) 
//	{
//		for (int i = 0; i < num_segments; i++)
//		{
//
//			verts[i * 9] = 0.f;
//			verts[i * 9 + 1] = 0.f;
//			verts[i * 9 + 2] = 0.f;
//
//			verts[i * 9 + 3] = (float)sin(DEG2RAD(current_angle)) * radius;
//			verts[i * 9 + 4] = (float)cos(DEG2RAD(current_angle)) * radius;
//			verts[i * 9 + 5] = 0.f;
//
//			current_angle += angle_offset;
//
//			verts[i * 9 + 6] = (float)sin(DEG2RAD(current_angle)) * radius;
//			verts[i * 9 + 7] = (float)cos(DEG2RAD(current_angle)) * radius;
//			verts[i * 9 + 8] = 0.f;
//		}
//	}
//
//	return verts;
//}
//
//std::vector<float> getVectorCircle(int num_segments, float radius)
//{
//	int n_verts = num_segments * 3; // 3 vertex per segment
//	int n_points = n_verts * 3; // 3 position values per vertex
//	int n_colors = n_verts * 3; // rgb colors per vertex
//	int n_texUV = n_verts * 2; // u,v texture coordinates per vertex
//
//	int element_size = n_points; // add new elements or attribute size here if needed
//
//	std::vector<float> circle;
//	
//	float angle_offset = 360.f / (float)num_segments;
//	float current_angle = 0.f;
//
//	for (int i = 0; i < num_segments; i++)
//	{
//
//		// circle origin
//		circle.push_back(0.f);
//		circle.push_back(0.f);
//		circle.push_back(0.f);
//
//		// circle ring vertex 1
//		circle.push_back((float)sin(DEG2RAD(current_angle)) * radius);
//		circle.push_back((float)cos(DEG2RAD(current_angle)) * radius);
//		circle.push_back(0.f);
//
//		// increment angle
//		current_angle += angle_offset;
//
//		// circle ring vertex 2
//		circle.push_back((float)sin(DEG2RAD(current_angle)) * radius);
//		circle.push_back((float)cos(DEG2RAD(current_angle)) * radius);
//		circle.push_back(0.f);
//	}
//
//	return circle;
//}
