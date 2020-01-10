# open game tools

Open game tools is a set of unencumbered, free, lightweight, easy-to-integrate tools for use in game development. 

So far it only contains the a MagicaVoxel scene reader, writer and merger [ogt_vox.h](https://github.com/jpaver/opengametools/blob/master/src/ogt_vox.h), but there will be more related tools to come.

Please consider contributing fixes, extensions, bug reports or feature requests to this project. If you have example scenes that fail to load or save correctly, feel free to send them to me and I'd be happy to investigate and make fixes for you.

See [CONTRIBUTING.md](https://github.com/jpaver/opengametools/blob/master/CONTRIBUTING.md) for more details.

## ogt_vox: MagicaVoxel scene reader, writer and merger

A C++ reader, writer and scene merger for [MagicaVoxel](https://ephtracy.github.io/)'s vox file format.

### Scene reading (.vox) 

With this library, reading a .vox file will result in a ogt_vox_scene structure which you use to:
- enumerate all or a subset of instance placements within a vox file.
- get the transforms for those instances as a matrix flattened relative to the scene file, or relative to their group hierarchy
- access the voxel model referenced by those placements ie. the 3d color grid.
- access the palette colors
- access layer, group and visible/hidden state for instances / groups.

I use this library to deep-reference models in .vox files and turn them into triangle meshes in my engine's importer/cooker. I also use it in an in-engine wizard to import all instance/scene data into your own engine scene format. This can allow your artists to use MagicaVoxel as a level editor, or simply a tool for managing a kit, palette of module-set to be used within levels or objects within your own editor. 

### Scene Writing (.vox)

The C++ writer for [MagicaVoxel](https://ephtracy.github.io/)'s vox file format takes a user-constructed ogt_vox_scene structure  and serializes it to .vox file format. When saved out to disk, it'll be loadable in MagicaVoxel. 

My own testing involved going through all of my on-disk vox files, and verified they load in MagicaVoxel 0.99.3-alpha. The files are usually smaller than the source .vox though because they will not preserve all the chunks within the original .vox file.

### Scene Merging (.vox)

You can load multiple .vox files simultaneously to obtain multiple ogt_vox_scene structures. You can then merge them into a single ogt_vox_scene and save it out.

If they have different palettes, the merger will do closest-color matches to try preserve the original colors in the output file.

If you find yourself needing to combine multiple files with very similar but non-identical palettes then this
is probably the tool for you. If you need to combine multiple files with completely different palettes that 
have only a few entries used, then this may also be for you! 

It should always work, but it may not give a good color fit if the voxel files you use most of their colors and have their own very different palettes. If you want to simply replace the merged file with a very specific palette, you can provide that to the merge function too. 

In the future, I might add a more holistic algorithm that prioritizes allocation to the final palette of colors based on their distance from other colors.

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

The reader, writer and merge functionaliy supports the most relevant chunks as described here:
- [MagicaVoxel-file-format-vox.txt](https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox.txt)
- [MagicaVoxel-file-format-vox-extension.txt](https://github.com/ephtracy/voxel-model/blob/master/MagicaVoxel-file-format-vox-extension.txt)

Though rOBJ/MATT/MATL chunks are not supported for now. If there is interest, I can look into it.

## Projects using open game tools

 I am using it in my own project. Details to be revealed soon!

## Acknowledgements

Thanks to [@ephtracy](https://twitter.com/ephtracy). So much would not have been possible without your generosity in releasing the tool free to the public, releasing information about the file format and patiently answering my DMs on twitter.

Thanks to [@nothings](https://twitter.com/nothings) for sharing your single-headers as well as the philosophy behind them. The latter has been invaluable to my personal growth.

Thanks to [@ndreca_com](https://twitter.com/ndreca_com) for feedback on the ogt_vox library!




