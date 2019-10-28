# open game tools

Open game tools is a set of unencumbered, free, lightweight, easy-to-integrate tools for use in game development. 

So far it only contains the a MagicaVoxel scene loader called ogt_vox_loader, but there is more to come when I have some cycles.

Please consider contributing. See [CONTRIBUTING.md](https://github.com/jpaver/opengametools/blob/master/CONTRIBUTING.md) for more details.

## MagicaVoxel scene loader

A C++ loader for [MagicaVoxel](https://ephtracy.github.io/)'s vox file format that allows you to deep access scene information within a .vox file.

- enumerate all or a subset of instance placements within a vox file.
- get the transforms for those instances as a matrix flattened relative to the scene file.
- access the voxel grid data for the models referenced by those placements.
- access the color/palette data for the grid.

With this data, you can write code to import all data from a .vox file into your own engine
scene format. This will allow your artists to use MagicaVoxel as a level editor, or simply 
a tool for managing a kit, palette of module-set to be used within levels or objects within 
your own editor. 

## Usage

See [demo/load_vox.cpp](https://github.com/jpaver/opengametools/blob/master/demo/load_vox.cpp) for a simple example.

1. Include ogt_vox_loader.h directly from one of your C or CPP files:

   ```c++
        #define OGT_VOX_IMPLEMENTATION
        #include "ogt_vox_loader.h"
   ```
   
2. Use your own file utilities to load a .vox file into a memory buffer. eg.

   ```c++
    FILE* fp = fopen("file.vox", "rb");
    uint32_t buffer_size = _filelength(_fileno(fp));
    uint8_t* buffer = new uint8_t[buffer_size];
    fread(buffer, buffer_size, 1, fp);
    fclose(fp);
   ```
	
3. Pass the buffer into ogt_vox_create_scene to get a scene object eg.

   ```c++
    const ogt_vox_scene* scene = ogt_vox_create_scene(buffer, buffer_size);
    // the buffer can be safely deleted once the scene is instantiated.
    delete[] buffer;
   ```
   
4. Now read all the information you need from the scene object and use it to import voxel data/instances. 
   eg. Here, I just print some basic information:

   ```c++
    printf("# models: %u\n",  scene->num_models);
    printf("# instances: %u\n", scene->num_instances);
   ```
	
5. Shut down the scene to release the memory it allocated. eg.

   ```c++
   ogt_vox_destroy_scene(scene);
   ```
## Note 

This currently supports most relevant chunks as described here:
- [MagicaVoxel-file-format-vox.txt](https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox.txt)
- [MagicaVoxel-file-format-vox-extension.txt](https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox-extension.txt)

.. though material information is not yet supported. 

## Projects using open game tools

 I am using it in my own project. Details to be revealed soon.

## Acknowledgements

Thanks to [@ephtracy](https://twitter.com/ephtracy). So much would not have been possible without your generosity in releasing the tool free to the public, releasing information about the file format and patiently answering my DMs on twitter.

Thanks to [@nothings](https://twitter.com/nothings) for sharing your single-headers as well as the philosophy behind them. The latter has been invaluable to my personal growth.




