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

#include "tinyply.h"
#include "MultiClassVoxelGrid.h"
#include "MultiClassVoxelizer.h"
#include "ColoredVoxelizer.h"
#include "ColoredVoxelGrid.h"

bool readPlyWithClass(std::string filepath, std::vector<Eigen::Vector3f> &vertices, std::vector<uint32_t> &faces, std::vector<uint8_t> &classes, std::vector<Eigen::Vector3i> &colormap) {

    std::ifstream ss(filepath, std::ios::binary);

    tinyply::PlyFile input_file(ss);

    std::vector<float> raw_vertices;
    std::vector<uint8_t> raw_colors;
    uint32_t num_vertices, num_colors, num_faces;
    num_vertices = num_colors = num_faces = 0;

    num_vertices = input_file.request_properties_from_element("vertex", { "x", "y", "z" }, raw_vertices);
    num_colors = input_file.request_properties_from_element("vertex", { "red", "green", "blue" }, raw_colors);
    num_faces = input_file.request_properties_from_element("face", { "vertex_indices" }, faces, 3);

    input_file.read(ss);

    vertices.resize(num_vertices);
    classes.resize(num_vertices);
    std::vector<Eigen::Vector3i> colors(num_vertices);

    int raw_vertices_i = 0;
    int raw_colors_i = 0;
    for (int i = 0; i < num_vertices; i++) {

        vertices[i][0] = raw_vertices[raw_vertices_i++];
        vertices[i][1] = raw_vertices[raw_vertices_i++];
        vertices[i][2] = raw_vertices[raw_vertices_i++];

        colors[i][0] = raw_colors[raw_colors_i++];
        colors[i][1] = raw_colors[raw_colors_i++];
        colors[i][2] = raw_colors[raw_colors_i++];

    }

    for (const auto& color : colors) {
        if(std::find(colormap.begin(), colormap.end(), color) == colormap.end())
            colormap.push_back(color);
    }

    if (colormap.size() > 255) {
        std::cerr << "Error: ClassyVoxelizer only supports up to 255 classes." << std::endl;
        return false;
    }

    int vertex_class_i = 0;
    for (const auto& color : colors) {

        int class_i;
        for (class_i = 0; class_i < colormap.size(); class_i++) {
            
            if (colormap[class_i] == color) {
                break;
            }
        }

        classes[vertex_class_i++] = class_i+1;
    }

    return true;

}

bool readPlyWithColor(std::string filepath, std::vector<Eigen::Vector3f> &vertices, std::vector<uint32_t> &faces, std::vector<Eigen::Vector3i> &colors) {

    std::ifstream ss(filepath, std::ios::binary);

    tinyply::PlyFile input_file(ss);

    std::vector<float> raw_vertices;
    std::vector<uint8_t> raw_colors;
    uint32_t num_vertices, num_colors, num_faces;
    num_vertices = num_colors = num_faces = 0;

    num_vertices = input_file.request_properties_from_element("vertex", { "x", "y", "z" }, raw_vertices);
    num_colors = input_file.request_properties_from_element("vertex", { "red", "green", "blue" }, raw_colors);
    num_faces = input_file.request_properties_from_element("face", { "vertex_indices" }, faces, 3);

    input_file.read(ss);

    vertices.resize(num_vertices);
    colors.resize(num_vertices);

    int raw_vertices_i = 0;
    int raw_colors_i = 0;
    for (int i = 0; i < num_vertices; i++) {

        vertices[i][0] = raw_vertices[raw_vertices_i++];
        vertices[i][1] = raw_vertices[raw_vertices_i++];
        vertices[i][2] = raw_vertices[raw_vertices_i++];

        colors[i][0] = raw_colors[raw_colors_i++];
        colors[i][1] = raw_colors[raw_colors_i++];
        colors[i][2] = raw_colors[raw_colors_i++];

    }

    // for (auto vertex : vertices)
    //     std::cout << std::to_string(vertex[0]) << " " << std::to_string(vertex[1]) << " " << std::to_string(vertex[2]) << std::endl;

    // for (auto face : faces)
    //     std::cout << std::to_string(face) << std::endl;

    // for (auto vertex_color : colors)
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
    
    std::string usage_message = "\nUsage:\n\n./classyvoxelizer <input> <output> <voxel_size> <class/color> <dense>\n\nCurrently only supports ASCII PLY with x, y, z, red, green, blue, alpha properties";
    
    if (argc < 6) {
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

    if (std::string(argv[4]) == "class") {
        readPlyWithClass(input_filepath, vertices, faces, vertex_classes, colormap);

    } else if (std::string(argv[4]) == "color") {
        readPlyWithColor(input_filepath, vertices, faces, colors);
    }

    bool dense = (std::string(argv[5]) == "true") ? true : false;

    Eigen::Vector3f min;
    Eigen::Vector3f max;
    getVoxelSpaceDimensions(vertices, voxel_size, min, max);

    std::cout << "Voxelizing at " << voxel_size << "m resolution: " << std::flush;

    if (std::string(argv[4]) == "class") {

        MultiClassVoxelGrid voxelgrid = MultiClassVoxelizer::voxelize(vertices, faces, vertex_classes, min, max, voxel_size);
        voxelgrid.saveAsPLY(output_filepath, colormap, dense);
    } else if (std::string(argv[4]) == "color") {
        
        ColoredVoxelGrid voxelgrid = ColoredVoxelizer::voxelize(vertices, faces, colors, min, max, voxel_size);
        voxelgrid.saveAsPLY(output_filepath, dense);
    }

    

    return 0;
}