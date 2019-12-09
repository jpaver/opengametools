# open game tools

Open game tools is a set of unencumbered, free, lightweight, easy-to-integrate tools for use in game development. 

So far it only contains the a MagicaVoxel scene reader, writer and merger [ogt_vox.h](https://github.com/jpaver/opengametools/blob/master/src/ogt_vox.h), but there will be more related tools to come.

Please consider contributing. See [CONTRIBUTING.md](https://github.com/jpaver/opengametools/blob/master/CONTRIBUTING.md) for more details.

## ogt_vox: MagicaVoxel scene reader, writer and merger

A C++ reader, writer and scene merger for [MagicaVoxel](https://ephtracy.github.io/)'s vox file format 

### Scene reading
Reading from .vox allows you to deep access scene information and:
- enumerate all or a subset of instance placements within a vox file.
- get the transforms for those instances as a matrix flattened relative to the scene file.
- access the voxel grid data for the models referenced by those placements.
- access the color/palette data for the grid.

With this data, you can write code to import all data from a .vox file into your own engine
scene format. This will allow your artists to use MagicaVoxel as a level editor, or simply 
a tool for managing a kit, palette of module-set to be used within levels or objects within 
your own editor. 

### Scene Writing
The C++ writer for [MagicaVoxel](https://ephtracy.github.io/)'s vox file format produces .vox 
files that are loadable in MagicaVoxel. The idea is to eventually allow manipulation of .vox 
files from other tools eg. scene exporter, procedural generator -> decoration workflows, 
merging and splitting .vox files procedurally. etc.

More work is incoming on the construction and merge API for the scene, right now the editability
of the scene is pretty bare-bones and dangerous.

I've tested loading multi-instance, multi-model scenes with transforms, layers, names using the 
scene reader, then written them out using the scene writer and verified they load within 
MagicaVoxel 0.99.3-alpha. The files are usually smaller than the source .vox and they will
not preserve all the chunks within the original .vox file, so are not binary identical.

If you have example scenes that fail to load or save correctly, I'd be happy to discretely 
investigate and make fixes.

### Scene Merging

The Scene merge code allows you to take multiple separate .vox files and put them all into a single vox file 
while doing some magic to try preserve the original colors in the output file.

If you find yourself needing to combine multiple files with very similar but non-identical palettes then this
is probably the tool for you. If you need to combine multiple files with completely different palettes that 
have only a few entries used, then this may also be for you!

It won't work for all data though. If you try to combine multiple vox files that use all the colors in their 
respective palettes and their palettes are very different, then your results will probably be bad.

## Usage

See [demo_vox.cpp](https://github.com/jpaver/opengametools/blob/master/demo/demo_vox.cpp) for a simple example.

1. Include ogt_vox.h directly from one of your C or CPP files:

   ```c++
        #define OGT_VOX_IMPLEMENTATION
        #include "ogt_vox.h"
   ```
   
2. Use your own file utilities to load a .vox file into a memory buffer. eg.

   ```c++
    FILE* fp = fopen("file.vox", "rb");
    uint32_t buffer_size = _filelength(_fileno(fp));
    uint8_t* buffer = new uint8_t[buffer_size];
    fread(buffer, buffer_size, 1, fp);
    fclose(fp);
   ```
	
3. Pass the buffer into ogt_vox_read_scene to get a scene object eg.

   ```c++
    const ogt_vox_scene* scene = ogt_vox_read_scene(buffer, buffer_size);
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

Both reader and writer currently support most relevant chunks as described here:
- [MagicaVoxel-file-format-vox.txt](https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox.txt)
- [MagicaVoxel-file-format-vox-extension.txt](https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox-extension.txt)

.. though material information is not yet supported. 

## Projects using open game tools

 I am using it in my own project. Details to be revealed soon.

## Acknowledgements

Thanks to [@ephtracy](https://twitter.com/ephtracy). So much would not have been possible without your generosity in releasing the tool free to the public, releasing information about the file format and patiently answering my DMs on twitter.

Thanks to [@nothings](https://twitter.com/nothings) for sharing your single-headers as well as the philosophy behind them. The latter has been invaluable to my personal growth.




