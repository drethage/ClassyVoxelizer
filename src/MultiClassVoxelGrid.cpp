/*
 Classy Voxelizer
 
 BSD 2-Clause License
 Copyright (c) 2018, Dario Rethage
 See LICENSE at package root for full license
 */
#include "MultiClassVoxelGrid.h"

MultiClassVoxelGrid::MultiClassVoxelGrid(Eigen::Vector3f grid_min, Eigen::Vector3f grid_max, float voxel_size) {
    
    _grid_min = grid_min;
    _grid_max = grid_max;
    _grid_size = grid_max - grid_min;
    _voxel_size = voxel_size;
	_voxels_per_dim = (_grid_size / voxel_size).cast<int>(); 
    _num_voxels = _voxels_per_dim.prod();
    _voxelgrid.resize(_num_voxels, 0);

}

int MultiClassVoxelGrid::getEnclosingVoxelID(Eigen::Vector3f vertex) {
    
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

bool MultiClassVoxelGrid::isVoxelOccupied(Eigen::Vector3f vertex) {
	
	uint32_t voxel_id = getEnclosingVoxelID(vertex);

	if (voxel_id == -1)
		return false;

	return isVoxelOccupied(voxel_id);

}

bool MultiClassVoxelGrid::isVoxelOccupied(uint32_t voxel_id) {
	return _voxelgrid[voxel_id] != 0;
}

void MultiClassVoxelGrid::setVoxelClass(uint32_t voxel_id, uint8_t class_i) {
    
    if(voxel_id < _num_voxels)
        _voxelgrid[voxel_id] = class_i;
    else
    	std::cout << "Invalid voxel id" << std::endl;
}

int MultiClassVoxelGrid::getVoxelClass(uint32_t voxel_id) {
    
    if(voxel_id < _num_voxels)
        return _voxelgrid[voxel_id];

    return -1;
}

Eigen::Vector3i MultiClassVoxelGrid::getVoxelsPerDim() {
    return _voxels_per_dim;
}

std::vector<uint8_t> MultiClassVoxelGrid::getVoxelGrid() {
    return _voxelgrid;
}

void MultiClassVoxelGrid::saveAsRAW(std::string filepath) {
    
    std::ofstream output_file(filepath);

	for (auto voxel : _voxelgrid)
		output_file << std::to_string(voxel) << std::endl;
    output_file.close();
}

void MultiClassVoxelGrid::saveAsPLYWithLabelProperties(std::string filepath, std::vector<Eigen::Vector3i> class_color_mapping, bool dense) {

	unsigned int num_alloc = _num_voxels;

	if (!dense) {
		num_alloc = 0;
		for (const auto& voxel : _voxelgrid) {
			if (voxel != 0)
				num_alloc++;
		}
	}
	std::vector<float> vertices(num_alloc * 3);
	std::vector<uint8_t> colors(num_alloc * 4);
	std::vector<uint8_t> classes(num_alloc);

	unsigned int raw_vertex_i = 0;
	unsigned int raw_color_i = 0;
	unsigned int raw_class_i = 0;
	for (int i = 0; i < _voxels_per_dim[0]; i++) {
		for (int j = 0; j < _voxels_per_dim[1]; j++) {
			for (int k = 0; k < _voxels_per_dim[2]; k++) {
				
				uint32_t voxel_id = (unsigned int) _voxels_per_dim[0] * _voxels_per_dim[1] * k + _voxels_per_dim[0] * j + i;
				
				if ( dense || (!dense && isVoxelOccupied(voxel_id))) {

					uint8_t class_i = getVoxelClass(voxel_id);

					Eigen::Vector3i color(255,255,255);
					if (!class_color_mapping.empty() && class_i != 0) {
						color = Eigen::Vector3i(class_color_mapping[class_i-1][0], class_color_mapping[class_i-1][1], class_color_mapping[class_i-1][2]);	
					}

					Eigen::Vector3f voxel_pos((i * _voxel_size) + _grid_min[0] + _voxel_size / 2, (j * _voxel_size) + _grid_min[1] + _voxel_size / 2, (k * _voxel_size) + _grid_min[2] + _voxel_size / 2);

					vertices[raw_vertex_i++] = voxel_pos[0];
					vertices[raw_vertex_i++] = voxel_pos[1];
					vertices[raw_vertex_i++] = voxel_pos[2];

					colors[raw_color_i++] = color[0];
					colors[raw_color_i++] = color[1];
					colors[raw_color_i++] = color[2];
					colors[raw_color_i++] = 255;

					classes[raw_class_i++] = class_i;

				}

			}
		}		
	}

	std::filebuf fb;
	fb.open(filepath, std::ios::out | std::ios::binary);
	std::ostream ss(&fb);

	tinyply::PlyFile out_file;
	out_file.add_properties_to_element("vertex", { "x", "y", "z" }, vertices);
	out_file.add_properties_to_element("vertex", { "label" }, classes);
	out_file.write(ss, true);
	fb.close();

}

void MultiClassVoxelGrid::saveAsPLY(std::string filepath, std::vector<Eigen::Vector3i> class_color_mapping, bool dense) {

	unsigned int num_alloc = _num_voxels;

	if (!dense) {
		num_alloc = 0;
		for (const auto& voxel : _voxelgrid) {
			if (voxel != 0)
				num_alloc++;
		}
	}
	std::vector<float> vertices(num_alloc * 3);
	std::vector<uint8_t> colors(num_alloc * 4);

	unsigned int raw_vertex_i = 0;
	unsigned int raw_color_i = 0;
	unsigned int raw_class_i = 0;
	for (int i = 0; i < _voxels_per_dim[0]; i++) {
		for (int j = 0; j < _voxels_per_dim[1]; j++) {
			for (int k = 0; k < _voxels_per_dim[2]; k++) {
				
				uint32_t voxel_id = (unsigned int) _voxels_per_dim[0] * _voxels_per_dim[1] * k + _voxels_per_dim[0] * j + i;
				
				if ( dense || (!dense && isVoxelOccupied(voxel_id))) {

					uint8_t class_i = getVoxelClass(voxel_id);

					Eigen::Vector3i color(255,255,255);
					if (class_i != 0) {
						color = Eigen::Vector3i(class_color_mapping[class_i-1][0], class_color_mapping[class_i-1][1], class_color_mapping[class_i-1][2]);	
					}

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

unsigned int MultiClassVoxelGrid::getNumOccupied() {
	
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