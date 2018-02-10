Classy Voxelizer
====
Voxelizes a mesh at a given voxel size using a face splitting algorithm. Classy Voxelizer can treat the colors of points as corresponding to classes and preserves these during voxelization. Or it can just treat them as colors to produce a colored voxelization.

![Teaser](img/teaser.png)

### Build:
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Usage:

For point clouds in which colors represent classes:
`./classy_voxelizer <input> <output> <voxel_size> class <dense>`

For point clouds in which colors don't represent classes:
`./classy_voxelizer <input> <output> <voxel_size> color <dense>`

### Notes:
* Reads ASCII/binary PLY, writes binary PLY (thanks to [tinyply](https://github.com/ddiakopoulos/tinyply))
* Produces dense (true voxelization) and sparse (uniform sampling) outputs
* <voxel_size> argument in meters
* Requires Eigen3


### License:
[BSD 2-Clause License](LICENSE)