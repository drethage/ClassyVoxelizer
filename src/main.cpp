/*
 Classy Voxelizer
 
 BSD 2-Clause License
 Copyright (c) 2018, Dario Rethage
 See LICENSE at package root for full license
 */

#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <set>

#include "MultiClassVoxelGrid.h"
#include "MultiClassVoxelizer.h"

bool readPly(std::string filepath, std::vector<Eigen::Vector3f> &vertices, std::vector<uint32_t> &faces, std::vector<uint8_t> &vertex_classes, std::vector<Eigen::Vector3i> &colormap) {

    std::ifstream input_file(filepath, std::ios::in | std::ios::binary);

    int num_vertices = 0;
    int num_faces = 0;
    std::string word;
    input_file >> word;
    while (word != "end_header") {
        if (word == "vertex") {
            input_file >> num_vertices;
        } else if (word == "face") {
            input_file >> num_faces;
        }
        input_file >> word;
    }

    vertices.resize(num_vertices);
    vertex_classes.resize(num_vertices);
    faces.resize(num_faces * 3);

    std::vector<Eigen::Vector3i> colors_list(num_vertices);

    int alpha = 0;
    for (int i = 0; i < num_vertices; i++) {

        input_file >> vertices[i][0];
        input_file >> vertices[i][1];
        input_file >> vertices[i][2];
        input_file >> colors_list[i][0];
        input_file >> colors_list[i][1];
        input_file >> colors_list[i][2];
        input_file >> alpha;

    }

    for (auto& color : colors_list) {
        if(std::find(colormap.begin(), colormap.end(), color) == colormap.end())
            colormap.push_back(color);
    }

    if (colormap.size() > 256) {
        std::cerr << "Error: MultiClassVoxelizer only supports up to 256 classes." << std::endl;
        return false;
    }

    int num_vertices_per_face = 0;
    int face_vertex_i = 0;

    for (int i = 0; i < num_faces; i++) {
        input_file >> num_vertices_per_face;
        input_file >> faces[face_vertex_i++];
        input_file >> faces[face_vertex_i++];
        input_file >> faces[face_vertex_i++];
    }

    int vertex_class_i = 0;
    for (auto color : colors_list) {

        int class_i;
        for (class_i = 0; class_i < colormap.size(); class_i++) {
            
            if (colormap[class_i] == color) {
                break;
            }
        }

        vertex_classes[vertex_class_i++] = class_i;
    }

    return true;

}


int main (int argc, char* argv[]) {
    
    std::string usage_message = "\nUsage:\n\n./classyvoxelizer <input> <output> <voxel_size>\n\nCurrently only supports ASCII PLY with x, y, z, red, green, blue, alpha properties";
    
    if (argc < 4) {
        std::cout << usage_message << std::endl;
        return 0;
    }

    std::vector<Eigen::Vector3f> vertices;
    std::vector<uint32_t> faces;
    std::vector<uint8_t> vertex_classes;
    std::vector<Eigen::Vector3i> colormap;

    std::string input_filepath = argv[1];
    std::string output_filepath = argv[2];
    double voxel_size = std::stod(argv[3]);
    
    readPly(input_filepath, vertices, faces, vertex_classes, colormap);

    Eigen::Vector3f centroid(0,0,0);

    for (auto vertex : vertices)
        centroid += vertex;

    centroid /= vertices.size();

    Eigen::Vector3f max_p = centroid;
    Eigen::Vector3f min_p = centroid;

    for (Eigen::Vector3f& vertex : vertices) {

        if ((vertex[0] - centroid[0]) > (max_p[0] - centroid[0]))
            max_p[0] = vertex[0];

        if ((vertex[1] - centroid[1]) > (max_p[1] - centroid[1]))
            max_p[1] = vertex[1];

        if ((vertex[2] - centroid[2]) > (max_p[2] - centroid[2]))
            max_p[2] = vertex[2];

        if ((vertex[0] - centroid[0]) < (min_p[0] - centroid[0]))
            min_p[0] = vertex[0];

        if ((vertex[1] - centroid[1]) < (min_p[1] - centroid[1]))
            min_p[1] = vertex[1];

        if ((vertex[2] - centroid[2]) < (min_p[2] - centroid[2]))
            min_p[2] = vertex[2];

    }

    max_p += Eigen::Vector3f(voxel_size, voxel_size, voxel_size);
    min_p -= Eigen::Vector3f(voxel_size, voxel_size, voxel_size);

    std::cout << "Voxelizing at " << voxel_size << "m resolution: " << std::flush;

    MultiClassVoxelGrid voxelgrid = MultiClassVoxelizer::voxelize(vertices, faces, vertex_classes, min_p, max_p, voxel_size);
    voxelgrid.saveAsPLY(output_filepath, colormap);

    return 0;
}