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
#include "ColoredVoxelizer.h"
#include "ColoredVoxelGrid.h"

bool readPlyWithClass(std::string filepath, std::vector<Eigen::Vector3f> &vertices, std::vector<uint32_t> &faces, std::vector<uint8_t> &vertex_classes, std::vector<Eigen::Vector3i> &colormap) {

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
    faces.resize(num_faces * 3);
    vertex_classes.resize(num_vertices);

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
        std::cerr << "Error: ClassyVoxelizer only supports up to 255 classes." << std::endl;
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

        vertex_classes[vertex_class_i++] = class_i+1;
    }

    return true;

}

bool readPlyWithColor(std::string filepath, std::vector<Eigen::Vector3f> &vertices, std::vector<uint32_t> &faces, std::vector<Eigen::Vector3i> &vertex_colors) {

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
    faces.resize(num_faces * 3);
    vertex_colors.resize(num_vertices);

    int alpha = 0;
    for (int i = 0; i < num_vertices; i++) {

        input_file >> vertices[i][0];
        input_file >> vertices[i][1];
        input_file >> vertices[i][2];
        input_file >> vertex_colors[i][0];
        input_file >> vertex_colors[i][1];
        input_file >> vertex_colors[i][2];
        input_file >> alpha;

    }

    int num_vertices_per_face = 0;
    int face_vertex_i = 0;

    for (int i = 0; i < num_faces; i++) {
        input_file >> num_vertices_per_face;
        input_file >> faces[face_vertex_i++];
        input_file >> faces[face_vertex_i++];
        input_file >> faces[face_vertex_i++];
    }

    // for (auto vertex : vertices)
    //     std::cout << std::to_string(vertex[0]) << " " << std::to_string(vertex[1]) << " " << std::to_string(vertex[2]) << std::endl;

    // for (auto face : faces)
    //     std::cout << std::to_string(face) << std::endl;

    // for (auto vertex_color : vertex_colors)
    //     std::cout << std::to_string(vertex_color[0]) << " " << std::to_string(vertex_color[1]) << " " << std::to_string(vertex_color[2]) << std::endl;

    return true;

}

void getVoxelSpaceDimensions(const std::vector<Eigen::Vector3f> &vertices, const double voxel_size, Eigen::Vector3f &min, Eigen::Vector3f &max) {

    Eigen::Vector3f centroid(0,0,0);

    for (const Eigen::Vector3f& vertex : vertices)
        centroid += vertex;

    centroid /= vertices.size();

    max = centroid;
    min = centroid;

    for (const Eigen::Vector3f& vertex : vertices) {

        if ((vertex[0] - centroid[0]) > (max[0] - centroid[0]))
            max[0] = vertex[0];

        if ((vertex[1] - centroid[1]) > (max[1] - centroid[1]))
            max[1] = vertex[1];

        if ((vertex[2] - centroid[2]) > (max[2] - centroid[2]))
            max[2] = vertex[2];

        if ((vertex[0] - centroid[0]) < (min[0] - centroid[0]))
            min[0] = vertex[0];

        if ((vertex[1] - centroid[1]) < (min[1] - centroid[1]))
            min[1] = vertex[1];

        if ((vertex[2] - centroid[2]) < (min[2] - centroid[2]))
            min[2] = vertex[2];

    }

    max += Eigen::Vector3f(voxel_size, voxel_size, voxel_size);
    min -= Eigen::Vector3f(voxel_size, voxel_size, voxel_size);

}


int main (int argc, char* argv[]) {
    
    std::string usage_message = "\nUsage:\n\n./classyvoxelizer <input> <output> <voxel_size> <class/color>\n\nCurrently only supports ASCII PLY with x, y, z, red, green, blue, alpha properties";
    
    if (argc < 5) {
        std::cout << usage_message << std::endl;
        return 0;
    }

    std::string input_filepath = argv[1];
    std::string output_filepath = argv[2];
    double voxel_size = std::stod(argv[3]);

    std::vector<Eigen::Vector3f> vertices;
    std::vector<uint32_t> faces;
    std::vector<uint8_t> vertex_classes;
    std::vector<Eigen::Vector3i> colormap;
    std::vector<Eigen::Vector3i> colors;

    bool ignore_classes = false;
    if (std::string(argv[4]) == "class") {
        readPlyWithClass(input_filepath, vertices, faces, vertex_classes, colormap);

    } else if (std::string(argv[4]) == "color") {
        readPlyWithColor(input_filepath, vertices, faces, colors);
    }

    Eigen::Vector3f min;
    Eigen::Vector3f max;
    getVoxelSpaceDimensions(vertices, voxel_size, min, max);

    std::cout << "Voxelizing at " << voxel_size << "m resolution: " << std::flush;

    if (std::string(argv[4]) == "class") {

        MultiClassVoxelGrid voxelgrid = MultiClassVoxelizer::voxelize(vertices, faces, vertex_classes, min, max, voxel_size);
        voxelgrid.saveAsPLY(output_filepath, colormap);
    } else if (std::string(argv[4]) == "color") {
        
        ColoredVoxelGrid voxelgrid = ColoredVoxelizer::voxelize(vertices, faces, colors, min, max, voxel_size);
        voxelgrid.saveAsPLY(output_filepath);
    }

    

    return 0;
}