/*
 Classy Voxelizer
 
 BSD 2-Clause License
 Copyright (c) 2018, Dario Rethage
 See LICENSE at package root for full license
 */

#include "ColoredVoxelizer.h"

ColoredVoxelGrid ColoredVoxelizer::voxelize(std::vector<Eigen::Vector3f> &vertices, std::vector<uint32_t> &faces, std::vector<Eigen::Vector3i> &colors, Eigen::Vector3f grid_min, Eigen::Vector3f grid_max, float voxel_size) {

    ColoredVoxelGrid voxel_grid(grid_min, grid_max, voxel_size);
    
    std::vector<uint32_t> split_faces;
    
    for (int i = 0; i < faces.size(); i+=3) {
        
        std::vector<uint32_t> face(3);
        face[0] = faces[i];
        face[1] = faces[i+1];
        face[2] = faces[i+2];
        
		std::vector<uint32_t> sub_faces;
		splitFace(voxel_grid, vertices, colors, face, sub_faces);
		split_faces.insert(split_faces.end(), sub_faces.begin(), sub_faces.end());
        
    }
    
    for (auto& split_face_vertex_i : split_faces) {

        int voxel_id = voxel_grid.getEnclosingVoxelID(vertices[split_face_vertex_i]);
        voxel_grid.setVoxelColor(voxel_id, colors[split_face_vertex_i]);
    }

    return voxel_grid;
    
}

void ColoredVoxelizer::splitFace(ColoredVoxelGrid &voxel_grid, std::vector<Eigen::Vector3f> &vertices, std::vector<Eigen::Vector3i> &colors, std::vector<uint32_t> &face, std::vector<uint32_t> &sub_faces) {

	if (areaOfTriangle(vertices[face[0]], vertices[face[1]], vertices[face[2]]) < ColoredVOXELIZER_MIN_TRIANGLE_AREA) {
		sub_faces.insert(sub_faces.end(), face.begin(), face.end());
		return;
	}

    std::vector<double> side_lengths(3,0);
    bool single_voxel_triangle = true;
    for (int i = 0; i < 3; i++) {
		if (voxel_grid.getEnclosingVoxelID(vertices[face[i % 3]]) != voxel_grid.getEnclosingVoxelID(vertices[face[(i + 1) % 3]])) {
            side_lengths[i] = euclideanDistance(vertices[face[i % 3]], vertices[face[(i + 1) % 3]]);
            single_voxel_triangle = false;
        }        
    }

    if (single_voxel_triangle) {
        sub_faces.insert(sub_faces.end(), face.begin(), face.end());
        return;
    }

    int longest_i = 0;
    double longest_length = 0;
    for (int i = 0; i < 3; i++) {
        if (side_lengths[i] > longest_length) {
            longest_length = side_lengths[i];
            longest_i = i;
        }
    }
    
	Eigen::Vector3f new_midpoint = getMidpoint(vertices[face[longest_i % 3]], vertices[face[(longest_i + 1) % 3]]);
    vertices.push_back(new_midpoint);
    colors.push_back( (colors[face[longest_i % 3]] + colors[face[(longest_i + 1) % 3]]) / 2 );
    
    std::vector<uint32_t> first_sub_face(3);
	first_sub_face[0] = face[longest_i % 3];
	first_sub_face[1] = face[(longest_i + 2) % 3];
    first_sub_face[2] = vertices.size()-1;
    
	std::vector<uint32_t> second_sub_face(3);
	second_sub_face[0] = face[(longest_i + 1) % 3];
	second_sub_face[1] = face[(longest_i + 2) % 3];
    second_sub_face[2] = vertices.size()-1;
    
    splitFace(voxel_grid, vertices, colors, first_sub_face, sub_faces);
    splitFace(voxel_grid, vertices, colors, second_sub_face, sub_faces);
            
}

float ColoredVoxelizer::areaOfTriangle(Eigen::Vector3f vertex_1, Eigen::Vector3f vertex_2, Eigen::Vector3f vertex_3) {

    return (vertex_2 - vertex_1).cross(vertex_3 - vertex_1).norm() / 2.0;

}

Eigen::Vector3f ColoredVoxelizer::getMidpoint(Eigen::Vector3f v1, Eigen::Vector3f v2) {
    
    return (v1 + v2) / 2;
}
           
float ColoredVoxelizer::euclideanDistance(Eigen::Vector3f v1, Eigen::Vector3f v2) {
    return std::sqrt(std::pow((v1[0] - v2[0]),2) + std::pow((v1[1] - v2[1]),2) + std::pow((v1[2] - v2[2]),2));
}