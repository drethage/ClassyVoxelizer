/*
 Classy Voxelizer
 
 BSD 2-Clause License
 Copyright (c) 2018, Dario Rethage
 See LICENSE at package root for full license
 */
#include "ColoredVoxelGrid.h"

ColoredVoxelGrid::ColoredVoxelGrid(Eigen::Vector3f grid_min, Eigen::Vector3f grid_max, float voxel_size) {
    
    _grid_min = grid_min;
    _grid_max = grid_max;
    _grid_size = grid_max - grid_min;
    std::cout << _grid_size[0] << " " << _grid_size[1] << " " << _grid_size[2] << std::endl;
    _voxel_size = voxel_size;
	_voxels_per_dim = (_grid_size / voxel_size).cast<int>();   
    _num_voxels = _voxels_per_dim.prod();

    std::cout << "Created voxel grid with " << _num_voxels << " voxels" << std::endl;
    _voxelgrid.resize(_num_voxels, Eigen::Vector3i(-1, -1, -1));

}

unsigned int ColoredVoxelGrid::getEnclosingVoxelID(Eigen::Vector3f vertex) {
    
    if (vertex[0] < _grid_min[0] ||
        vertex[1] < _grid_min[1] ||
        vertex[2] < _grid_min[2] ||
        vertex[0] > _grid_max[0] ||
        vertex[1] > _grid_max[1] ||
        vertex[2] > _grid_max[2])
        return -1;
    
    Eigen::Vector3f vertex_offset_discretized = (vertex - _grid_min) / _voxel_size;
    
    return (unsigned int) _voxels_per_dim[0] * _voxels_per_dim[1] * std::floor(vertex_offset_discretized[2]) + _voxels_per_dim[0] * std::floor(vertex_offset_discretized[1]) + std::floor(vertex_offset_discretized[0]);
    
}

bool ColoredVoxelGrid::isVoxelOccupied(Eigen::Vector3f vertex) {
	
	uint32_t voxel_id = getEnclosingVoxelID(vertex);

	if (voxel_id == -1)
		return false;

	return isVoxelOccupied(voxel_id);

}

bool ColoredVoxelGrid::isVoxelOccupied(uint32_t voxel_id) {
	return (_voxelgrid[voxel_id][0] != -1 || _voxelgrid[voxel_id][1] != -1 || _voxelgrid[voxel_id][2] != -1) ? true : false;
}

void ColoredVoxelGrid::setVoxelColor(uint32_t voxel_id, Eigen::Vector3i color) {
    
    if(voxel_id < _num_voxels)
        _voxelgrid[voxel_id] = color;
    // else
    // 	std::cout << "Invalid voxel id" << std::endl;
}

Eigen::Vector3i ColoredVoxelGrid::getVoxelColor(uint32_t voxel_id) {
    
    if(voxel_id < _num_voxels)
        return _voxelgrid[voxel_id];

    return Eigen::Vector3i(-1,-1,-1);
}

Eigen::Vector3i ColoredVoxelGrid::getVoxelsPerDim() {
    return _voxels_per_dim;
}

std::vector<Eigen::Vector3i> ColoredVoxelGrid::getVoxelGrid() {
    return _voxelgrid;
}

void ColoredVoxelGrid::saveAsRAW(std::string filepath) {
    
    std::ofstream output_file(filepath);

	for (auto voxel : _voxelgrid)
		output_file << std::to_string(voxel[0]) << " " << std::to_string(voxel[1]) << " " << std::to_string(voxel[2]) << std::endl;
    output_file.close();
}

void ColoredVoxelGrid::saveAsPLY(std::string filepath) {

	std::ofstream output_file(filepath);

	long num_occupied_voxels = 0;
	for (auto voxel : _voxelgrid) {
		if (voxel != Eigen::Vector3i(-1, -1, -1))
			num_occupied_voxels++;
	}

	output_file << "ply" << std::endl;
    output_file << "format ascii 1.0" << std::endl;
    output_file << "element vertex " << num_occupied_voxels << std::endl;
    output_file << "property float x" << std::endl;
    output_file << "property float y" << std::endl;
    output_file << "property float z" << std::endl;
    output_file << "property uchar red" << std::endl;
    output_file << "property uchar green" << std::endl;
    output_file << "property uchar blue" << std::endl;
	output_file << "property uchar alpha" << std::endl;
	output_file << "end_header" << std::endl;

	for (int i = 0; i < _voxels_per_dim[0]; i++) {
		for (int j = 0; j < _voxels_per_dim[1]; j++) {
			for (int k = 0; k < _voxels_per_dim[2]; k++) {
				
				uint32_t voxel_id = (unsigned int) _voxels_per_dim[0] * _voxels_per_dim[1] * k + _voxels_per_dim[0] * j + i;
				
				if (isVoxelOccupied(voxel_id)) {

					Eigen::Vector3i color = getVoxelColor(voxel_id);

					Eigen::Vector3f voxel_pos((i * _voxel_size) + _grid_min[0] + _voxel_size / 2, (j * _voxel_size) + _grid_min[1] + _voxel_size / 2, (k * _voxel_size) + _grid_min[2] + _voxel_size / 2);

					output_file << voxel_pos[0] << " " << voxel_pos[1] << " " << voxel_pos[2] << " "
            << color[0] << " " << color[1] << " " << color[2] << " " << "255" << std::endl;

				}

			}
		}		
	}
	
	output_file.close();

}

unsigned int ColoredVoxelGrid::getNumOccupied() {
	
	unsigned int numOccupied = 0;
	for (int i = 0; i < _voxels_per_dim[0]; i++) {
		for (int j = 0; j < _voxels_per_dim[1]; j++) {
			for (int k = 0; k < _voxels_per_dim[2]; k++) {

				uint32_t voxel_id = (unsigned int) _voxels_per_dim[0] * _voxels_per_dim[1] * k + _voxels_per_dim[0] * j + i;

				if (isVoxelOccupied(voxel_id))
					numOccupied++;
			}
		}		
	}

	return numOccupied;
}