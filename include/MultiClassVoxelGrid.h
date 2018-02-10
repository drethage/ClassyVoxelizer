/*
 Classy Voxelizer
 
 BSD 2-Clause License
 Copyright (c) 2018, Dario Rethage
 See LICENSE at package root for full license
 */

#ifndef __MULTICLASSVOXELGRID__
#define __MULTICLASSVOXELGRID__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>

//Eigen
#include <Eigen/Dense>

#include "tinyply.h"

class MultiClassVoxelGrid {
public:
    MultiClassVoxelGrid(Eigen::Vector3f grid_min, Eigen::Vector3f grid_max, float voxel_size);
    int getEnclosingVoxelID(Eigen::Vector3f vertex);
    Eigen::Vector3i getVoxelsPerDim();
    void setVoxelClass(uint32_t voxel_id, uint8_t class_i);
    int getVoxelClass(uint32_t voxel_id);
    std::vector<uint8_t> getVoxelGrid();
    void saveAsRAW(std::string filepath);
    void saveAsPLY(std::string filepath, std::vector<Eigen::Vector3i> class_color_mapping, bool dense);
	bool isVoxelOccupied(uint32_t voxel_id);
	bool isVoxelOccupied(Eigen::Vector3f vertex);
	unsigned int getNumOccupied();
    
private:
    Eigen::Vector3i _voxels_per_dim;
    Eigen::Vector3f _grid_min;
    Eigen::Vector3f _grid_max;
    Eigen::Vector3f _grid_size;
    float _voxel_size;
    std::vector<uint8_t> _voxelgrid;
    uint32_t _num_voxels;
    
};

#endif /* defined(__MULTICLASSVOXELGRID__) */
