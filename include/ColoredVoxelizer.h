/*
 Classy Voxelizer
 
 BSD 2-Clause License
 Copyright (c) 2018, Dario Rethage
 See LICENSE at package root for full license
 */

#ifndef __ColoredVOXELIZER__
#define __ColoredVOXELIZER__

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <string>

//Eigen
#include <Eigen/Dense>

#include "ColoredVoxelGrid.h"

#define ColoredVOXELIZER_MIN_TRIANGLE_AREA 0.00001

class ColoredVoxelizer {
public:
    static ColoredVoxelGrid voxelize(std::vector<Eigen::Vector3f> &vertices, std::vector<uint32_t> &faces, std::vector<Eigen::Vector3i> &colors, Eigen::Vector3f grid_min, Eigen::Vector3f grid_max, float voxel_size);
    
private:
    static Eigen::Vector3f getMidpoint(Eigen::Vector3f v1, Eigen::Vector3f v2);
    static float euclideanDistance(Eigen::Vector3f v1, Eigen::Vector3f v2);
    static float areaOfTriangle(Eigen::Vector3f vertex_1, Eigen::Vector3f vertex_2, Eigen::Vector3f vertex_3);
	static void splitFace(ColoredVoxelGrid &voxel_grid, std::vector<Eigen::Vector3f> &vertices, std::vector<Eigen::Vector3i> &colors, std::vector<uint32_t> &face, std::vector<uint32_t> &sub_faces);
       
};

#endif /* defined(__ColoredVOXELIZER__) */
