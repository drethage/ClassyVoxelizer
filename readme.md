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

`./classy_voxelizer <input> <output> <voxel_size> <class/color>`

### Notes:
* Currently only supports ASCII-encoded PLY
* Assumes each vertex has the following properties in order: x, y, z, red, green, blue, alpha
* <voxel_size> argument in meters
* Requires Eigen3


### License
[BSD 2-Clause License](LICENSE)