/*
 Classy Voxelizer
 
 BSD 2-Clause License
 Copyright (c) 2018, Dario Rethage
 See LICENSE at package root for full license
 */

#ifndef __ColoredVOXELGRID__
#define __ColoredVOXELGRID__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>

//Eigen
#include <Eigen/Dense>

class ColoredVoxelGrid {
public:
    ColoredVoxelGrid(Eigen::Vector3f grid_min, Eigen::Vector3f grid_max, float voxel_size);
    uint32_t getEnclosingVoxelID(Eigen::Vector3f vertex);
    Eigen::Vector3i getVoxelsPerDim();
    void setVoxelColor(uint32_t voxel_id, Eigen::Vector3i color);
    Eigen::Vector3i getVoxelColor(uint32_t voxel_id);
    std::vector<Eigen::Vector3i> getVoxelGrid();
    void saveAsRAW(std::string filepath);
    void saveAsPLY(std::string filepath);
	bool isVoxelOccupied(uint32_t voxel_id);
	bool isVoxelOccupied(Eigen::Vector3f vertex);
	unsigned int getNumOccupied();
    
private:
    Eigen::Vector3i _voxels_per_dim;
    Eigen::Vector3f _grid_min;
    Eigen::Vector3f _grid_max;
    Eigen::Vector3f _grid_size;
    float _voxel_size;
    std::vector<Eigen::Vector3i> _voxelgrid;
    uint32_t _num_voxels;
    
};

#endif /* defined(__ColoredVOXELGRID__) */
