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
    _voxel_size = voxel_size;
	_voxels_per_dim = (_grid_size / voxel_size).cast<int>();   
    _num_voxels = _voxels_per_dim.prod();
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

	unsigned int num_occupied_voxels = 0;
	for (const auto& voxel : _voxelgrid) {
		if (voxel != Eigen::Vector3i(-1, -1, -1))
			num_occupied_voxels++;
	}

	std::vector<float> vertices(num_occupied_voxels * 3);
	std::vector<uint8_t> colors(num_occupied_voxels * 4);

	unsigned int raw_vertex_i = 0;
	unsigned int raw_color_i = 0;
	for (int i = 0; i < _voxels_per_dim[0]; i++) {
		for (int j = 0; j < _voxels_per_dim[1]; j++) {
			for (int k = 0; k < _voxels_per_dim[2]; k++) {
				
				uint32_t voxel_id = (unsigned int) _voxels_per_dim[0] * _voxels_per_dim[1] * k + _voxels_per_dim[0] * j + i;
				
				if (isVoxelOccupied(voxel_id)) {

					Eigen::Vector3i color = getVoxelColor(voxel_id);
					Eigen::Vector3f voxel_pos((i * _voxel_size) + _grid_min[0] + _voxel_size / 2, (j * _voxel_size) + _grid_min[1] + _voxel_size / 2, (k * _voxel_size) + _grid_min[2] + _voxel_size / 2);

					vertices[raw_vertex_i++] = voxel_pos[0];
					vertices[raw_vertex_i++] = voxel_pos[1];
					vertices[raw_vertex_i++] = voxel_pos[2];

					colors[raw_color_i++] = color[0];
					colors[raw_color_i++] = color[1];
					colors[raw_color_i++] = color[2];
					colors[raw_color_i++] = 255;

				}

			}
		}		
	}


	std::filebuf fb;
	fb.open(filepath, std::ios::out | std::ios::binary);
	std::ostream ss(&fb);

	tinyply::PlyFile out_file;

	out_file.add_properties_to_element("vertex", { "x", "y", "z" }, vertices);
	out_file.add_properties_to_element("vertex", { "red", "green", "blue", "alpha" }, colors);
	out_file.write(ss, true);

	fb.close();

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