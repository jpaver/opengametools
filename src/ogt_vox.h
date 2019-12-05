/*
    opengametools vox file reader/writer - v0.6 - MIT license - Justin Paver, Oct 2019

    This is a single-header-file library that provides easy-to-use
    support for reading MagicaVoxel .vox files into structures that
    are easy to dereference and extract information from. It also
    supports writing back out to .vox file from those structures.

    Please see the MIT license information at the end of this file.

    Also, please consider sharing any improvements you make.

    For more information and more tools, visit:
      https://github.com/jpaver/opengametools

    USAGE (See demo_vox.cpp)

    1.  To compile this library, do this in *one* C or C++ file:
        #define OGT_VOX_IMPLEMENTATION
        #include "ogt_vox.h"

    2. load a .vox file into a memory buffer. 
       
    3. construct a scene from the memory buffer:
       ogt_vox_scene* scene = ogt_vox_read_scene(buffer, buffer_size);

    4. use the scene members to acquire the information you need. eg.
       printf("# of layers: %u\n", scene->num_layers );

    EXPLANATION

    A ogt_vox_scene comprises primarily a set of instances, models, layers and a palette.

    A ogt_vox_palette contains a set of 256 colors that is used for the scene.
    Each color is represented by a 4-tuple called an ogt_vox_rgba which contains red,
    green, blue and alpha values for the color.

    A ogt_vox_model is a 3-dimensional grid of voxels, where each of those voxels
    is represented by an 8-bit color index. Voxels are arranged in order of increasing
    X then increasing Y then increasing Z.

    Given the x,y,z values for a voxel within the model dimensions, the voxels index
    in the grid can be obtained as follows:

        voxel_index = x + (y * model->size_x) + (z * model->size_x * model->size_y)

    The index is only valid if the coordinate x,y,z satisfy the following conditions:
            0 <= x < model->size_x -AND-
            0 <= y < model->size_y -AND-
            0 <= z < model->size_z

    A voxels color index can be obtained as follows:

        uint8_t color_index = model->voxel_data[voxel_index];

    If color_index == 0, the voxel is not solid and can be skipped,
    If color_index != 0, the voxel is solid and can be used to lookup the color in the palette:

        ogt_vox_rgba color = scene->palette.color[ color_index]

    A ogt_vox_instance is an individual placement of a voxel model within the scene. Each
    instance has a transform that determines its position and orientation within the scene,
    but it also has an index that specifies which model the instance uses for its shape. It
    is expected that there is a many-to-one mapping of instances to models.

    An ogt_vox_layer is used to conceptually group instances. Each instance indexes the
    layer that it belongs to, but the layer itself has its own name and hidden/shown state.
*/
#ifndef OGT_VOX_H__
#define OGT_VOX_H__

#if _MSC_VER == 1400	
    // VS2005 doesn't have inttypes or stdint so we just define what we need here.
    typedef unsigned char uint8_t;
    typedef signed int    int32_t;
    typedef unsigned int  uint32_t;
    #define UINT32_MAX	0xFFFFFFFF
#else
    #include <inttypes.h>
#endif

    // color
    struct ogt_vox_rgba
    {
        uint8_t r,g,b,a;            // red, green, blue and alpha components of a color.
    };

    // column-major 4x4 matrix
    struct ogt_vox_transform 
    {
        float m00, m01, m02, m03;   // column 0 of 4x4 matrix, 1st three elements = x axis vector, last element always 0.0
        float m10, m11, m12, m13;   // column 1 of 4x4 matrix, 1st three elements = y axis vector, last element always 0.0
        float m20, m21, m22, m23;   // column 2 of 4x4 matrix, 1st three elements = z axis vector, last element always 0.0
        float m30, m31, m32, m33;   // column 3 of 4x4 matrix. 1st three elements = translation vector, last element always 1.0
    };

    // a palette of colors
    struct ogt_vox_palette
    {
        ogt_vox_rgba color[256];      // palette of colors. use the voxel indices to lookup color from the palette.
    };

    // a 3-dimensional model of voxels
    struct ogt_vox_model
    {
        uint32_t       size_x;        // number of voxels in the local x dimension
        uint32_t       size_y;        // number of voxels in the local y dimension
        uint32_t       size_z;        // number of voxels in the local z dimension
        uint32_t       voxel_hash;    // hash of the content of the grid.
        const uint8_t* voxel_data;    // grid of voxel data comprising color indices in x -> y -> z order. a color index of 0 means empty, all other indices mean solid and can be used to index the scene's palette to obtain the color for the voxel.
    };

    // an instance of a model within the scene
    struct ogt_vox_instance
    {
        const char*       name;         // name of the instance if there is one, will be NULL otherwise.
        ogt_vox_transform transform;    // orientation and position of this instance within the scene.
        uint32_t          model_index;  // index of the model used by this instance. used to lookup the model in the scene's models[] array.
        uint32_t          layer_index;  // index of the layer used by this instance. used to lookup the layer in the scene's layers[] array.
        bool              hidden;       // whether this instance is individually hidden or not
    };

    // describes a layer within the scene
    struct ogt_vox_layer
    {
        const char* name;               // name of this layer if there is one, will be NULL otherwise.
        bool        hidden;             // whether this layer is hidden or not.
    };

    // the scene parsed from a .vox file.
    struct ogt_vox_scene
    {
        uint32_t                num_models;     // number of models within the scene.
        uint32_t                num_instances;  // number of instances in the scene
        uint32_t                num_layers;     // number of layers in the scene
        const ogt_vox_model**   models;         // array of models. size is num_models
        const ogt_vox_instance* instances;      // array of instances. size is num_instances
        const ogt_vox_layer*    layers;         // array of layers. size is num_layers
        ogt_vox_palette         palette;        // the palette for this scene
    };

    // allocate memory function interface. pass in size, and get a pointer to memory with at least that size available.
    typedef void* (*ogt_vox_alloc_func)(size_t size);

    // free memory function interface. pass in a pointer previously allocated and it will be released back to the system managing memory.
    typedef void  (*ogt_vox_free_func)(void* ptr);

    // override the default scene memory allocator if you need to control memory precisely.
    void ogt_vox_set_memory_allocator(ogt_vox_alloc_func alloc_func, ogt_vox_free_func free_func);

    // creates a scene from a vox file within a memory buffer of a given size.
    // you can destroy the input buffer once you have the scene as this function will allocate separate memory for the scene objecvt.
    const ogt_vox_scene* ogt_vox_read_scene(const uint8_t* buffer, uint32_t buffer_size);

    // destroys a scene object to release its memory.
    void ogt_vox_destroy_scene(const ogt_vox_scene* scene);

    // writes the scene to a new buffer and returns the buffer size. free the buffer with ogt_vox_free
    uint8_t* ogt_vox_write_scene(const ogt_vox_scene* scene, uint32_t& buffer_size);

    // frees a buffer returned by ogt_vox_write_scene
    void     ogt_vox_free(void* mem);

#endif // OGT_VOX_H__

//-----------------------------------------------------------------------------------------------------------------
//
// If you're only interested in using this library, everything you need is above this point.
// If you're interested in how this library works, everything you need is below this point.
//
//-----------------------------------------------------------------------------------------------------------------
#ifdef OGT_VOX_IMPLEMENTATION
    #include <assert.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdio.h>
    
    // MAKE_VOX_CHUNK_ID: used to construct a literal to describe a chunk in a .vox file.
    #define MAKE_VOX_CHUNK_ID(c0,c1,c2,c3)     ( (c0<<0) | (c1<<8) | (c2<<16) | (c3<<24) )

    static const uint32_t CHUNK_ID_VOX_ = MAKE_VOX_CHUNK_ID('V','O','X',' ');
    static const uint32_t CHUNK_ID_MAIN = MAKE_VOX_CHUNK_ID('M','A','I','N');
    static const uint32_t CHUNK_ID_SIZE = MAKE_VOX_CHUNK_ID('S','I','Z','E');
    static const uint32_t CHUNK_ID_XYZI = MAKE_VOX_CHUNK_ID('X','Y','Z','I');
    static const uint32_t CHUNK_ID_RGBA = MAKE_VOX_CHUNK_ID('R','G','B','A');
    static const uint32_t CHUNK_ID_nTRN = MAKE_VOX_CHUNK_ID('n','T','R','N');
    static const uint32_t CHUNK_ID_nGRP = MAKE_VOX_CHUNK_ID('n','G','R','P');
    static const uint32_t CHUNK_ID_nSHP = MAKE_VOX_CHUNK_ID('n','S','H','P');
    static const uint32_t CHUNK_ID_IMAP = MAKE_VOX_CHUNK_ID('I','M','A','P');
    static const uint32_t CHUNK_ID_LAYR = MAKE_VOX_CHUNK_ID('L','A','Y','R');
    static const uint32_t CHUNK_ID_MATL = MAKE_VOX_CHUNK_ID('M','A','T','L');
    static const uint32_t CHUNK_ID_MATT = MAKE_VOX_CHUNK_ID('M','A','T','T');

    // Some older .vox files will not store a palette, in which case the following palette will be used!
    static const uint8_t k_default_vox_palette[256 * 4] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcc, 0xff, 0xff, 0xff, 0x99, 0xff, 0xff, 0xff, 0x66, 0xff, 0xff, 0xff, 0x33, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xcc, 0xff, 0xff, 0xff, 0xcc, 0xcc, 0xff,
        0xff, 0xcc, 0x99, 0xff, 0xff, 0xcc, 0x66, 0xff, 0xff, 0xcc, 0x33, 0xff, 0xff, 0xcc, 0x00, 0xff, 0xff, 0x99, 0xff, 0xff, 0xff, 0x99, 0xcc, 0xff, 0xff, 0x99, 0x99, 0xff, 0xff, 0x99, 0x66, 0xff,
        0xff, 0x99, 0x33, 0xff, 0xff, 0x99, 0x00, 0xff, 0xff, 0x66, 0xff, 0xff, 0xff, 0x66, 0xcc, 0xff, 0xff, 0x66, 0x99, 0xff, 0xff, 0x66, 0x66, 0xff, 0xff, 0x66, 0x33, 0xff, 0xff, 0x66, 0x00, 0xff,
        0xff, 0x33, 0xff, 0xff, 0xff, 0x33, 0xcc, 0xff, 0xff, 0x33, 0x99, 0xff, 0xff, 0x33, 0x66, 0xff, 0xff, 0x33, 0x33, 0xff, 0xff, 0x33, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xcc, 0xff,
        0xff, 0x00, 0x99, 0xff, 0xff, 0x00, 0x66, 0xff, 0xff, 0x00, 0x33, 0xff, 0xff, 0x00, 0x00, 0xff, 0xcc, 0xff, 0xff, 0xff, 0xcc, 0xff, 0xcc, 0xff, 0xcc, 0xff, 0x99, 0xff, 0xcc, 0xff, 0x66, 0xff,
        0xcc, 0xff, 0x33, 0xff, 0xcc, 0xff, 0x00, 0xff, 0xcc, 0xcc, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0xff, 0xcc, 0xcc, 0x99, 0xff, 0xcc, 0xcc, 0x66, 0xff, 0xcc, 0xcc, 0x33, 0xff, 0xcc, 0xcc, 0x00, 0xff,
        0xcc, 0x99, 0xff, 0xff, 0xcc, 0x99, 0xcc, 0xff, 0xcc, 0x99, 0x99, 0xff, 0xcc, 0x99, 0x66, 0xff, 0xcc, 0x99, 0x33, 0xff, 0xcc, 0x99, 0x00, 0xff, 0xcc, 0x66, 0xff, 0xff, 0xcc, 0x66, 0xcc, 0xff,
        0xcc, 0x66, 0x99, 0xff, 0xcc, 0x66, 0x66, 0xff, 0xcc, 0x66, 0x33, 0xff, 0xcc, 0x66, 0x00, 0xff, 0xcc, 0x33, 0xff, 0xff, 0xcc, 0x33, 0xcc, 0xff, 0xcc, 0x33, 0x99, 0xff, 0xcc, 0x33, 0x66, 0xff,
        0xcc, 0x33, 0x33, 0xff, 0xcc, 0x33, 0x00, 0xff, 0xcc, 0x00, 0xff, 0xff, 0xcc, 0x00, 0xcc, 0xff, 0xcc, 0x00, 0x99, 0xff, 0xcc, 0x00, 0x66, 0xff, 0xcc, 0x00, 0x33, 0xff, 0xcc, 0x00, 0x00, 0xff,
        0x99, 0xff, 0xff, 0xff, 0x99, 0xff, 0xcc, 0xff, 0x99, 0xff, 0x99, 0xff, 0x99, 0xff, 0x66, 0xff, 0x99, 0xff, 0x33, 0xff, 0x99, 0xff, 0x00, 0xff, 0x99, 0xcc, 0xff, 0xff, 0x99, 0xcc, 0xcc, 0xff,
        0x99, 0xcc, 0x99, 0xff, 0x99, 0xcc, 0x66, 0xff, 0x99, 0xcc, 0x33, 0xff, 0x99, 0xcc, 0x00, 0xff, 0x99, 0x99, 0xff, 0xff, 0x99, 0x99, 0xcc, 0xff, 0x99, 0x99, 0x99, 0xff, 0x99, 0x99, 0x66, 0xff,
        0x99, 0x99, 0x33, 0xff, 0x99, 0x99, 0x00, 0xff, 0x99, 0x66, 0xff, 0xff, 0x99, 0x66, 0xcc, 0xff, 0x99, 0x66, 0x99, 0xff, 0x99, 0x66, 0x66, 0xff, 0x99, 0x66, 0x33, 0xff, 0x99, 0x66, 0x00, 0xff,
        0x99, 0x33, 0xff, 0xff, 0x99, 0x33, 0xcc, 0xff, 0x99, 0x33, 0x99, 0xff, 0x99, 0x33, 0x66, 0xff, 0x99, 0x33, 0x33, 0xff, 0x99, 0x33, 0x00, 0xff, 0x99, 0x00, 0xff, 0xff, 0x99, 0x00, 0xcc, 0xff,
        0x99, 0x00, 0x99, 0xff, 0x99, 0x00, 0x66, 0xff, 0x99, 0x00, 0x33, 0xff, 0x99, 0x00, 0x00, 0xff, 0x66, 0xff, 0xff, 0xff, 0x66, 0xff, 0xcc, 0xff, 0x66, 0xff, 0x99, 0xff, 0x66, 0xff, 0x66, 0xff,
        0x66, 0xff, 0x33, 0xff, 0x66, 0xff, 0x00, 0xff, 0x66, 0xcc, 0xff, 0xff, 0x66, 0xcc, 0xcc, 0xff, 0x66, 0xcc, 0x99, 0xff, 0x66, 0xcc, 0x66, 0xff, 0x66, 0xcc, 0x33, 0xff, 0x66, 0xcc, 0x00, 0xff,
        0x66, 0x99, 0xff, 0xff, 0x66, 0x99, 0xcc, 0xff, 0x66, 0x99, 0x99, 0xff, 0x66, 0x99, 0x66, 0xff, 0x66, 0x99, 0x33, 0xff, 0x66, 0x99, 0x00, 0xff, 0x66, 0x66, 0xff, 0xff, 0x66, 0x66, 0xcc, 0xff,
        0x66, 0x66, 0x99, 0xff, 0x66, 0x66, 0x66, 0xff, 0x66, 0x66, 0x33, 0xff, 0x66, 0x66, 0x00, 0xff, 0x66, 0x33, 0xff, 0xff, 0x66, 0x33, 0xcc, 0xff, 0x66, 0x33, 0x99, 0xff, 0x66, 0x33, 0x66, 0xff,
        0x66, 0x33, 0x33, 0xff, 0x66, 0x33, 0x00, 0xff, 0x66, 0x00, 0xff, 0xff, 0x66, 0x00, 0xcc, 0xff, 0x66, 0x00, 0x99, 0xff, 0x66, 0x00, 0x66, 0xff, 0x66, 0x00, 0x33, 0xff, 0x66, 0x00, 0x00, 0xff,
        0x33, 0xff, 0xff, 0xff, 0x33, 0xff, 0xcc, 0xff, 0x33, 0xff, 0x99, 0xff, 0x33, 0xff, 0x66, 0xff, 0x33, 0xff, 0x33, 0xff, 0x33, 0xff, 0x00, 0xff, 0x33, 0xcc, 0xff, 0xff, 0x33, 0xcc, 0xcc, 0xff,
        0x33, 0xcc, 0x99, 0xff, 0x33, 0xcc, 0x66, 0xff, 0x33, 0xcc, 0x33, 0xff, 0x33, 0xcc, 0x00, 0xff, 0x33, 0x99, 0xff, 0xff, 0x33, 0x99, 0xcc, 0xff, 0x33, 0x99, 0x99, 0xff, 0x33, 0x99, 0x66, 0xff,
        0x33, 0x99, 0x33, 0xff, 0x33, 0x99, 0x00, 0xff, 0x33, 0x66, 0xff, 0xff, 0x33, 0x66, 0xcc, 0xff, 0x33, 0x66, 0x99, 0xff, 0x33, 0x66, 0x66, 0xff, 0x33, 0x66, 0x33, 0xff, 0x33, 0x66, 0x00, 0xff,
        0x33, 0x33, 0xff, 0xff, 0x33, 0x33, 0xcc, 0xff, 0x33, 0x33, 0x99, 0xff, 0x33, 0x33, 0x66, 0xff, 0x33, 0x33, 0x33, 0xff, 0x33, 0x33, 0x00, 0xff, 0x33, 0x00, 0xff, 0xff, 0x33, 0x00, 0xcc, 0xff,
        0x33, 0x00, 0x99, 0xff, 0x33, 0x00, 0x66, 0xff, 0x33, 0x00, 0x33, 0xff, 0x33, 0x00, 0x00, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xcc, 0xff, 0x00, 0xff, 0x99, 0xff, 0x00, 0xff, 0x66, 0xff,
        0x00, 0xff, 0x33, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xcc, 0xff, 0xff, 0x00, 0xcc, 0xcc, 0xff, 0x00, 0xcc, 0x99, 0xff, 0x00, 0xcc, 0x66, 0xff, 0x00, 0xcc, 0x33, 0xff, 0x00, 0xcc, 0x00, 0xff,
        0x00, 0x99, 0xff, 0xff, 0x00, 0x99, 0xcc, 0xff, 0x00, 0x99, 0x99, 0xff, 0x00, 0x99, 0x66, 0xff, 0x00, 0x99, 0x33, 0xff, 0x00, 0x99, 0x00, 0xff, 0x00, 0x66, 0xff, 0xff, 0x00, 0x66, 0xcc, 0xff,
        0x00, 0x66, 0x99, 0xff, 0x00, 0x66, 0x66, 0xff, 0x00, 0x66, 0x33, 0xff, 0x00, 0x66, 0x00, 0xff, 0x00, 0x33, 0xff, 0xff, 0x00, 0x33, 0xcc, 0xff, 0x00, 0x33, 0x99, 0xff, 0x00, 0x33, 0x66, 0xff,
        0x00, 0x33, 0x33, 0xff, 0x00, 0x33, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xcc, 0xff, 0x00, 0x00, 0x99, 0xff, 0x00, 0x00, 0x66, 0xff, 0x00, 0x00, 0x33, 0xff, 0xee, 0x00, 0x00, 0xff,
        0xdd, 0x00, 0x00, 0xff, 0xbb, 0x00, 0x00, 0xff, 0xaa, 0x00, 0x00, 0xff, 0x88, 0x00, 0x00, 0xff, 0x77, 0x00, 0x00, 0xff, 0x55, 0x00, 0x00, 0xff, 0x44, 0x00, 0x00, 0xff, 0x22, 0x00, 0x00, 0xff,
        0x11, 0x00, 0x00, 0xff, 0x00, 0xee, 0x00, 0xff, 0x00, 0xdd, 0x00, 0xff, 0x00, 0xbb, 0x00, 0xff, 0x00, 0xaa, 0x00, 0xff, 0x00, 0x88, 0x00, 0xff, 0x00, 0x77, 0x00, 0xff, 0x00, 0x55, 0x00, 0xff,
        0x00, 0x44, 0x00, 0xff, 0x00, 0x22, 0x00, 0xff, 0x00, 0x11, 0x00, 0xff, 0x00, 0x00, 0xee, 0xff, 0x00, 0x00, 0xdd, 0xff, 0x00, 0x00, 0xbb, 0xff, 0x00, 0x00, 0xaa, 0xff, 0x00, 0x00, 0x88, 0xff,
        0x00, 0x00, 0x77, 0xff, 0x00, 0x00, 0x55, 0xff, 0x00, 0x00, 0x44, 0xff, 0x00, 0x00, 0x22, 0xff, 0x00, 0x00, 0x11, 0xff, 0xee, 0xee, 0xee, 0xff, 0xdd, 0xdd, 0xdd, 0xff, 0xbb, 0xbb, 0xbb, 0xff,
        0xaa, 0xaa, 0xaa, 0xff, 0x88, 0x88, 0x88, 0xff, 0x77, 0x77, 0x77, 0xff, 0x55, 0x55, 0x55, 0xff, 0x44, 0x44, 0x44, 0xff, 0x22, 0x22, 0x22, 0xff, 0x11, 0x11, 0x11, 0xff, 0x00, 0x00, 0x00, 0xff,
    };

    // internal math/helper utilities
    static inline uint32_t _vox_max(uint32_t a, uint32_t b) { 
        return (a > b) ? a : b; 
    }
    static inline uint32_t _vox_min(uint32_t a, uint32_t b) { 
        return (a < b) ? a : b; 
    }

    // string utilities
    #ifdef _MSC_VER
        #define _vox_str_scanf(str,...)      sscanf_s(str,__VA_ARGS__)
        #define _vox_strcpy_static(dst,src)  strcpy_s(dst,src)
        #define _vox_strcasecmp(a,b)         _stricmp(a,b)
        #define _vox_strcmp(a,b)             strcmp(a,b)
        #define _vox_strlen(a)               strlen(a)
        #define _vox_sprintf(str,str_max,fmt,...)    sprintf_s(str, str_max, fmt, __VA_ARGS__)
    #else
        #define _vox_str_scanf(str,...)      scanf_s(str,__VA_ARGS__)
        #define _vox_strcpy_static(dst,src)  strcpy(dst,src)
        #define _vox_strcasecmp(a,b)         strcasecmp(a,b)
        #define _vox_strcmp(a,b)             strcmp(a,b)
        #define _vox_strlen(a)               strlen(a)
    #endif

    // 3d vector utilities
    struct vec3 {
        float x, y, z;
    };
    static inline vec3 vec3_make(float x, float y, float z) { vec3 v; v.x = x; v.y = y; v.z = z; return v; }
    static inline vec3 vec3_negate(const vec3& v) { vec3 r; r.x = -v.x;  r.y = -v.y; r.z = -v.z; return r; }

    // API for emulating file transactions on an in-memory buffer of data.
    struct _vox_file {
        const  uint8_t* buffer;       // source buffer data
        const uint32_t  buffer_size;  // size of the data in the buffer
        uint32_t        offset;       // current offset in the buffer data.
    };

    static bool _vox_file_read(_vox_file* fp, void* data, uint32_t data_size) {
        size_t data_to_read = _vox_min(fp->buffer_size - fp->offset, data_size);
        memcpy(data, &fp->buffer[fp->offset], data_to_read);
        fp->offset += data_size;
        return data_to_read == data_size;
    }

    static void _vox_file_seek_forwards(_vox_file* fp, uint32_t offset) {
        fp->offset += offset;
    }

    static bool _vox_file_eof(const _vox_file* fp) {
        return fp->offset >= fp->buffer_size;
    }

    static const void* _vox_file_data_pointer(const _vox_file* fp) {
        return &fp->buffer[fp->offset];
    }

    // hash utilities
    static uint32_t _vox_hash(const uint8_t* data, uint32_t data_size) {
        unsigned long hash = 0;
        for (uint32_t i = 0; i < data_size; i++)
            hash = data[i] + (hash * 65559);
        return hash;
    }

    // memory allocation utils.
    static void* _ogt_priv_alloc_default(size_t size) { return malloc(size); }
    static void  _ogt_priv_free_default(void* pPtr)    { free(pPtr); }
    static ogt_vox_alloc_func g_alloc_func = _ogt_priv_alloc_default; // default function for allocating 
    static ogt_vox_free_func  g_free_func = _ogt_priv_free_default;   // default  function for freeing.

    // set the provided allocate/free functions if they are non-null, otherwise reset to default allocate/free functions
    void ogt_vox_set_memory_allocator(ogt_vox_alloc_func alloc_func, ogt_vox_free_func free_func)
    {
        assert((alloc_func && free_func) ||      // both alloc/free must be non-NULL -OR-
            (!alloc_func && !free_func));    // both alloc/free must be NULL. No mixing 'n matching.
        if (alloc_func && free_func) {
            g_alloc_func = alloc_func;
            g_free_func = free_func;
        }
        else  {
            // reset to default allocate/free functions.
            g_alloc_func = _ogt_priv_alloc_default;
            g_free_func = _ogt_priv_free_default;
        }
    }

    static void* _vox_malloc(size_t iSize) {
        return iSize ? g_alloc_func(iSize) : NULL;
    }

    static void* _vox_calloc(size_t iSize) {
        void* pMem = _vox_malloc(iSize);
        if (pMem)
            memset(pMem, 0, iSize);
        return pMem;
    }

    static void _vox_free(void* old_ptr) {
        if (old_ptr)
            g_free_func(old_ptr);
    }

    static void* _vox_realloc(void* old_ptr, size_t old_size, size_t new_size) {
        // early out if new size is non-zero and no resize is required.
        if (new_size && old_size >= new_size)
            return old_ptr;

        // memcpy from the old ptr only if both sides are valid.
        void* new_ptr = _vox_malloc(new_size);
        if (new_ptr) {
            // copy any existing elements over
            if (old_ptr && old_size)
                memcpy(new_ptr, old_ptr, old_size);
            // zero out any new tail elements
            assert(new_size > old_size); // this should be guaranteed by the _vox_realloc early out case above.
            uintptr_t new_tail_ptr = (uintptr_t)new_ptr + old_size;
            memset((void*)new_tail_ptr, 0, new_size - old_size);
        }
        if (old_ptr)
            _vox_free(old_ptr);
        return new_ptr;
    }

    // std::vector-style allocator, which use client-provided allocation functions.
    template <class T> struct _vox_array {
        _vox_array() : data(NULL), capacity(0), count(0) { }
        ~_vox_array() {
            _vox_free(data);
            data = NULL;
            count = 0;
            capacity = 0;
        }
        void reserve(size_t new_capacity) {
            data = (T*)_vox_realloc(data, capacity * sizeof(T), new_capacity * sizeof(T));
            capacity = new_capacity;
        }
        void grow_to_fit_index(size_t index) {
            if (index >= count)
                resize(index + 1);
        }
        void resize(size_t new_count) {
            if (new_count > capacity)
                reserve(new_count);
            count = new_count;
        }
        void push_back(const T & new_element) {
            if (count == capacity) {
                size_t new_capacity = capacity ? (capacity * 3) >> 1 : 2;   // grow by 50% each time, otherwise start at 2 elements.
                reserve(new_capacity);
                assert(capacity > count);
            }
            data[count++] = new_element;
        }
        void push_back_many(const T * new_elements, size_t num_elements) {
            if (count + num_elements > capacity) {
                size_t new_capacity = capacity + num_elements;
                new_capacity = new_capacity ? (new_capacity * 3) >> 1 : 2;   // grow by 50% each time, otherwise start at 2 elements.
                reserve(new_capacity);
                assert(capacity >= (count + num_elements));
            }
            for (size_t i = 0; i < num_elements; i++)
                data[count + i] = new_elements[i];
            count += num_elements;
        }
        size_t size() const {
            return count;
        }
        T& operator[](size_t index) {
            assert(index < count);
            return data[index];
        }
        const T& operator[](size_t index) const {
            assert(index < count);
            return data[index];
        }
        T*     data;      // data for the array
        size_t capacity;  // capacity of the array
        size_t count;      // size of the array
    };

    // matrix utilities
    static ogt_vox_transform _vox_transform_identity() {
        ogt_vox_transform t;
        t.m00 = 1.0f; t.m01 = 0.0f; t.m02 = 0.0f; t.m03 = 0.0f;
        t.m10 = 0.0f; t.m11 = 1.0f; t.m12 = 0.0f; t.m13 = 0.0f;
        t.m20 = 0.0f; t.m21 = 0.0f; t.m22 = 1.0f; t.m23 = 0.0f;
        t.m30 = 0.0f; t.m31 = 0.0f; t.m32 = 0.0f; t.m33 = 1.0f;
        return t;
    }

    static ogt_vox_transform _vox_transform_multiply(const ogt_vox_transform& a, const ogt_vox_transform& b) {
        ogt_vox_transform r;
        r.m00 = (a.m00 * b.m00) + (a.m01 * b.m10) + (a.m02 * b.m20) + (a.m03 * b.m30);
        r.m01 = (a.m00 * b.m01) + (a.m01 * b.m11) + (a.m02 * b.m21) + (a.m03 * b.m31);
        r.m02 = (a.m00 * b.m02) + (a.m01 * b.m12) + (a.m02 * b.m22) + (a.m03 * b.m32);
        r.m03 = (a.m00 * b.m03) + (a.m01 * b.m13) + (a.m02 * b.m23) + (a.m03 * b.m33);
        r.m10 = (a.m10 * b.m00) + (a.m11 * b.m10) + (a.m12 * b.m20) + (a.m13 * b.m30);
        r.m11 = (a.m10 * b.m01) + (a.m11 * b.m11) + (a.m12 * b.m21) + (a.m13 * b.m31);
        r.m12 = (a.m10 * b.m02) + (a.m11 * b.m12) + (a.m12 * b.m22) + (a.m13 * b.m32);
        r.m13 = (a.m10 * b.m03) + (a.m11 * b.m13) + (a.m12 * b.m23) + (a.m13 * b.m33);
        r.m20 = (a.m20 * b.m00) + (a.m21 * b.m10) + (a.m22 * b.m20) + (a.m23 * b.m30);
        r.m21 = (a.m20 * b.m01) + (a.m21 * b.m11) + (a.m22 * b.m21) + (a.m23 * b.m31);
        r.m22 = (a.m20 * b.m02) + (a.m21 * b.m12) + (a.m22 * b.m22) + (a.m23 * b.m32);
        r.m23 = (a.m20 * b.m03) + (a.m21 * b.m13) + (a.m22 * b.m23) + (a.m23 * b.m33);
        r.m30 = (a.m30 * b.m00) + (a.m31 * b.m10) + (a.m32 * b.m20) + (a.m33 * b.m30);
        r.m31 = (a.m30 * b.m01) + (a.m31 * b.m11) + (a.m32 * b.m21) + (a.m33 * b.m31);
        r.m32 = (a.m30 * b.m02) + (a.m31 * b.m12) + (a.m32 * b.m22) + (a.m33 * b.m32);
        r.m33 = (a.m30 * b.m03) + (a.m31 * b.m13) + (a.m32 * b.m23) + (a.m33 * b.m33);
        return r;
    }

    // dictionary utilities
    static const uint32_t k_vox_max_dict_buffer_size = 4096;
    static const uint32_t k_vox_max_dict_key_value_pairs = 256;
    struct _vox_dictionary {
        const char* keys[k_vox_max_dict_key_value_pairs];
        const char* values[k_vox_max_dict_key_value_pairs];
        uint32_t    num_key_value_pairs;
        char        buffer[k_vox_max_dict_buffer_size + 4];    // max 4096, +4 for safety
        uint32_t    buffer_mem_used;
    };

    static bool _vox_file_read_dict(_vox_dictionary * dict, _vox_file * fp) {
        uint32_t num_pairs_to_read = 0;
        _vox_file_read(fp, &num_pairs_to_read, sizeof(uint32_t));
        assert(num_pairs_to_read <= k_vox_max_dict_key_value_pairs);

        dict->buffer_mem_used = 0;
        dict->num_key_value_pairs = 0;
        for (uint32_t i = 0; i < num_pairs_to_read; i++) {
            // get the size of the key string
            uint32_t key_string_size = 0;
            _vox_file_read(fp, &key_string_size, sizeof(uint32_t));
            // allocate space for the key, and read it in.
            if (dict->buffer_mem_used + key_string_size > k_vox_max_dict_buffer_size)
                return false;
            char* key = &dict->buffer[dict->buffer_mem_used];
            dict->buffer_mem_used += key_string_size + 1;    // + 1 for zero terminator
            _vox_file_read(fp, key, key_string_size);
            key[key_string_size] = 0;    // zero-terminate
            assert(_vox_strlen(key) == key_string_size);    // sanity check

            // get the size of the value string
            uint32_t value_string_size = 0;
            _vox_file_read(fp, &value_string_size, sizeof(uint32_t));
            // allocate space for the value, and read it in.
            if (dict->buffer_mem_used + value_string_size > k_vox_max_dict_buffer_size)
                return (false);
            char* value = &dict->buffer[dict->buffer_mem_used];
            dict->buffer_mem_used += value_string_size + 1;    // + 1 for zero terminator
            _vox_file_read(fp, value, value_string_size);
            value[value_string_size] = 0;    // zero-terminate
            assert(_vox_strlen(value) == value_string_size);    // sanity check
            // now assign it in the dictionary
            dict->keys[dict->num_key_value_pairs] = key;
            dict->values[dict->num_key_value_pairs] = value;
            dict->num_key_value_pairs++;
        }

        return true;
    }

    // helper for looking up in the dictionary
    static const char* _vox_dict_get_value_as_string(const _vox_dictionary* dict, const char* key_to_find, const char* default_value = NULL) {
        for (uint32_t i = 0; i < dict->num_key_value_pairs; i++)
            if (_vox_strcasecmp(dict->keys[i], key_to_find) == 0)
                return dict->values[i];
        return default_value;
    }

    static ogt_vox_transform _vox_make_transform_from_dict_strings(const char* rotation_string, const char* translation_string) {
        ogt_vox_transform transform = _vox_transform_identity();

        if (rotation_string != NULL) {
            static vec3 k_vectors[4] = {
                vec3_make(1.0f, 0.0f, 0.0f),
                vec3_make(0.0f, 1.0f, 0.0f),
                vec3_make(0.0f, 0.0f, 1.0f),
                vec3_make(0.0f, 0.0f, 0.0f)    // invalid!
            };

            static const uint32_t k_row2_index[] = { ~0ul, ~0ul, ~0ul, 2, ~0ul, 1, 0, ~0ul};

            // compute the per-row indexes into k_vectors[] array.
            // unpack rotation bits. 
            //  bits  : meaning
            //  0 - 1 : index of the non-zero entry in the first row
            //  2 - 3 : index of the non-zero entry in the second row
            uint32_t packed_rotation_bits = atoi(rotation_string);
            uint32_t row0_vec_index = (packed_rotation_bits >> 0) & 3;
            uint32_t row1_vec_index = (packed_rotation_bits >> 2) & 3;
            uint32_t row2_vec_index = k_row2_index[(1 << row0_vec_index) | (1 << row1_vec_index)];    // process of elimination to determine row 2 index based on row0/row1 being one of {0,1,2} choose 2.
            assert(row2_vec_index != ~0ul);

            // unpack rotation bits for vector signs
            //  bits  : meaning
            //  4     : the sign in the first row  (0 : positive; 1 : negative)
            //  5     : the sign in the second row (0 : positive; 1 : negative)
            //  6     : the sign in the third row  (0 : positive; 1 : negative)
            vec3 row0 = k_vectors[row0_vec_index];
            vec3 row1 = k_vectors[row1_vec_index];
            vec3 row2 = k_vectors[row2_vec_index];
            if (packed_rotation_bits & (1 << 4))
                row0 = vec3_negate(row0);
            if (packed_rotation_bits & (1 << 5))
                row1 = vec3_negate(row1);
            if (packed_rotation_bits & (1 << 6))
                row2 = vec3_negate(row2);

            // magicavoxel stores rows, we need columns, so we do the swizzle here into columns
            transform.m00 = row0.x; transform.m01 = row1.x; transform.m02 = row2.x;
            transform.m10 = row0.y; transform.m11 = row1.y; transform.m12 = row2.y;
            transform.m20 = row0.z; transform.m21 = row1.z; transform.m22 = row2.z;
        }

        if (translation_string != NULL) {
            int32_t x = 0;
            int32_t y = 0;
            int32_t z = 0;
            _vox_str_scanf(translation_string, "%i %i %i", &x, &y, &z);
            transform.m30 = (float)x;
            transform.m31 = (float)y;
            transform.m32 = (float)z;
        }
        return transform;
    }

    enum _vox_scene_node_type
    {
        k_nodetype_invalid   = 0,    // has not been parsed yet.
        k_nodetype_group     = 1,
        k_nodetype_transform = 2,
        k_nodetype_shape     = 3,
    };

    struct _vox_scene_node_ {
        _vox_scene_node_type node_type;    // only gets assigned when this has been parsed, otherwise will be k_nodetype_invalid
        union {
            // used only when node_type == k_nodetype_transform
            struct {
                char              name[64];    // max name size is 64
                ogt_vox_transform transform;
                uint32_t          child_node_id;
                uint32_t          layer_id;
                bool              hidden;
            } transform;
            // used only when node_type == k_nodetype_group
            struct {
                uint32_t first_child_node_id_index; // the index of the first child node ID within the ChildNodeID array
                uint32_t num_child_nodes;           // number of child node IDs starting at the first index
            } group;
            // used only when node_type == k_nodetype_shape
            struct {
                uint32_t model_id;                  // will be UINT32_MAX if there is no model. Unlikely, there should always be a model.
            } shape;
        } u;
    };

    static void generate_instances_for_node(
        const _vox_array<_vox_scene_node_> & nodes, uint32_t node_index, const _vox_array<uint32_t> & child_id_array, uint32_t layer_index, 
        const ogt_vox_transform& transform, const _vox_array<ogt_vox_model*> & model_ptrs, const char* transform_last_name, bool transform_last_hidden,
        _vox_array<ogt_vox_instance> & instances, _vox_array<char> & string_data)
    {
        const _vox_scene_node_* node = &nodes[node_index];
        assert(node);
        switch (node->node_type) 
        {
            case k_nodetype_transform: 
            {
                ogt_vox_transform new_transform = _vox_transform_multiply(node->u.transform.transform, transform);    // child transform * parent transform
                const char* new_transform_name = node->u.transform.name[0] ? node->u.transform.name : NULL;
                transform_last_name = transform_last_name ? transform_last_name : new_transform_name;    // if there was already a transform name, keep that otherwise keep the new transform name
                generate_instances_for_node(nodes, node->u.transform.child_node_id, child_id_array, node->u.transform.layer_id, new_transform, model_ptrs, transform_last_name, node->u.transform.hidden, instances, string_data);
                break;
            }
            case k_nodetype_group: 
            {
                const uint32_t* child_node_ids = (const uint32_t*)& child_id_array[node->u.group.first_child_node_id_index];
                for (uint32_t i = 0; i < node->u.group.num_child_nodes; i++) {
                    generate_instances_for_node(nodes, child_node_ids[i], child_id_array, layer_index, transform, model_ptrs, transform_last_name, transform_last_hidden, instances, string_data);
                }
                break;
            }
            case k_nodetype_shape: 
            {
                assert(node->u.shape.model_id < model_ptrs.size());
                if (node->u.shape.model_id < model_ptrs.size() &&    // model ID is valid
                    model_ptrs[node->u.shape.model_id] != NULL )     // model is non-NULL.   
                {
                    ogt_vox_instance new_instance;
                    new_instance.model_index = node->u.shape.model_id;
                    new_instance.transform   = transform;
                    new_instance.layer_index = layer_index;
                    new_instance.hidden      = transform_last_hidden;
                    // if we got a transform name, allocate space in string_data for it and keep track of the index 
                    // within string data. This will be patched to a real pointer at the very end.
                    new_instance.name = 0;
                    if (transform_last_name && transform_last_name[0]) {
                        new_instance.name = (const char*)(string_data.size());
                        size_t name_size = _vox_strlen(transform_last_name) + 1;       // +1 for terminator
                        string_data.push_back_many(transform_last_name, name_size);
                    }
                    // create the instance
                    instances.push_back(new_instance);
                }
                break;
            }
            default:
            {
                assert(0); // unhandled node type!
            }
        }
    }

    // ensure instances are ordered in increasing order of model_idnex
    static int _vox_ordered_compare_instance(const void* _lhs, const void* _rhs) {
        const ogt_vox_instance* lhs = (const ogt_vox_instance*)_lhs;
        const ogt_vox_instance* rhs = (const ogt_vox_instance*)_rhs;
        return lhs->model_index < rhs->model_index ? -1 :
               lhs->model_index > rhs->model_index ?  1 : 0;
    }
    
    // returns true if the 2 models are content-wise identical.
    static bool _vox_models_are_equal(const ogt_vox_model* lhs, const ogt_vox_model* rhs) {
        // early out: if hashes don't match, they can't be equal
        // if hashes match, they might be equal OR there might be a hash collision.
        if (lhs->voxel_hash != rhs->voxel_hash)
            return false;
        // early out: if number of voxels in the model's grid don't match, they can't be equal.
        uint32_t num_voxels_lhs = lhs->size_x * lhs->size_y * lhs->size_z;
        uint32_t num_voxels_rhs = rhs->size_x * rhs->size_y * rhs->size_z;
        if (num_voxels_lhs != num_voxels_rhs)
            return false;
        // Finally, we know their hashes are the same, and their dimensions are the same
        // but they are only equal if they have exactly the same voxel data.
        return memcmp(lhs->voxel_data, rhs->voxel_data, num_voxels_lhs) == 0 ? true : false;
    }

    const ogt_vox_scene* ogt_vox_read_scene(const uint8_t * buffer, uint32_t bufferSize) {
        _vox_file file = { buffer, bufferSize, 0 };
        _vox_file* fp = &file;

        // parsing state/context
        _vox_array<ogt_vox_model*>    model_ptrs;
        _vox_array<_vox_scene_node_> nodes;
        _vox_array<ogt_vox_instance> instances;
        _vox_array<char>             string_data;
        _vox_array<ogt_vox_layer>    layers;
        _vox_array<uint32_t>         child_ids;
        ogt_vox_palette              palette;
        _vox_dictionary              dict;
        uint32_t                     size_x = 0;
        uint32_t                     size_y = 0;
        uint32_t                     size_z = 0;
        uint8_t                      index_map[256];
        bool                         found_index_map_chunk = false;

        // size some of our arrays to prevent resizing during the parsing for smallish cases.
        model_ptrs.reserve(64);
        instances.reserve(256);
        child_ids.reserve(256);
        nodes.reserve(16);
        layers.reserve(8);
        string_data.reserve(256);

        // push a sentinel character into these datastructures. This allows us to keep indexes 
        // rather than pointers into data-structures that grow, and still allow an index of 0 
        // to means invalid
        string_data.push_back('X');
        child_ids.push_back(-1);

        // copy the default palette into the scene. It may get overwritten by a palette chunk later
        memcpy(&palette, k_default_vox_palette, sizeof(ogt_vox_palette));

        // load and validate fileheader and file version.
        uint32_t file_header;
        uint32_t file_version;
        _vox_file_read(fp, &file_header, sizeof(uint32_t));
        _vox_file_read(fp, &file_version, sizeof(uint32_t));
        if (file_header != CHUNK_ID_VOX_ || file_version != 150)
            return NULL;

        // parse chunks until we reach the end of the file/buffer
        while (!_vox_file_eof(fp))
        {
            // read the fields common to all chunks
            uint32_t chunk_id         = 0;
            uint32_t chunk_size       = 0;
            uint32_t chunk_child_size = 0;
            _vox_file_read(fp, &chunk_id, sizeof(uint32_t));
            _vox_file_read(fp, &chunk_size, sizeof(uint32_t));
            _vox_file_read(fp, &chunk_child_size, sizeof(uint32_t));

            // process the chunk.
            switch (chunk_id)
            {
                case CHUNK_ID_MAIN:
                {
                    assert(chunk_size == 0);
                    break;
                }
                case CHUNK_ID_SIZE:
                {
                    assert(chunk_size == 12 && chunk_child_size == 0);
                    _vox_file_read(fp, &size_x, sizeof(uint32_t));
                    _vox_file_read(fp, &size_y, sizeof(uint32_t));
                    _vox_file_read(fp, &size_z, sizeof(uint32_t));
                    break;
                }
                case CHUNK_ID_XYZI:
                {
                    assert(chunk_child_size == 0 && size_x && size_y && size_z);    // must have read a SIZE chunk prior to XYZI.
                    // read the number of voxels to process for this moodel
                    uint32_t num_voxels_in_chunk = 0;
                    _vox_file_read(fp, &num_voxels_in_chunk, sizeof(uint32_t));
                    if (num_voxels_in_chunk != 0) {
                        uint32_t voxel_count = size_x * size_y * size_z;
                        ogt_vox_model * model = (ogt_vox_model*)_vox_calloc(sizeof(ogt_vox_model) + voxel_count);        // 1 byte for each voxel
                        if (!model)
                            return NULL;
                        uint8_t * voxel_data = (uint8_t*)&model[1];

                        // insert the model into the model array
                        model_ptrs.push_back(model);

                        // now setup the model
                        model->size_x = size_x;
                        model->size_y = size_y;
                        model->size_z = size_z;
                        model->voxel_data = voxel_data;

                        // setup some strides for computing voxel index based on x/y/z
                        const uint32_t k_stride_x = 1;
                        const uint32_t k_stride_y = size_x;
                        const uint32_t k_stride_z = size_x * size_y;

                        // read this many voxels and store it in voxel data.
                        const uint8_t * packed_voxel_data = (const uint8_t*)_vox_file_data_pointer(fp);
                        for (uint32_t i = 0; i < num_voxels_in_chunk; i++) {
                            uint8_t x = packed_voxel_data[i * 4 + 0];
                            uint8_t y = packed_voxel_data[i * 4 + 1];
                            uint8_t z = packed_voxel_data[i * 4 + 2];
                            uint8_t colorIdx = packed_voxel_data[i * 4 + 3];
                            assert(x < size_x && y < size_y && z < size_z);
                            voxel_data[(x * k_stride_x) + (y * k_stride_y) + (z * k_stride_z)] = colorIdx;
                        }
                        _vox_file_seek_forwards(fp, num_voxels_in_chunk * 4);
                        // compute the hash of the voxels in this model-- used to accelerate duplicate models checking.
                        model->voxel_hash = _vox_hash(voxel_data, size_x * size_y * size_z);
                    }
                    else {
                        model_ptrs.push_back(NULL);
                    }
                    break;
                }
                case CHUNK_ID_RGBA:
                {
                    assert(chunk_size == sizeof(palette));
                    _vox_file_read(fp, &palette, sizeof(palette));
                    break;
                }
                case CHUNK_ID_nTRN:
                {
                    uint32_t node_id;
                    _vox_file_read(fp, &node_id, sizeof(node_id));

                    // Parse the node dictionary, which can contain:
                    //   _name:   string
                    //   _hidden: 0/1
                    char node_name[64];
                    bool hidden = false;
                    node_name[0] = 0;
                    {
                        _vox_file_read_dict(&dict, fp);
                        const char* name_string = _vox_dict_get_value_as_string(&dict, "_name");
                        if (name_string)
                            _vox_strcpy_static(node_name, name_string);
                        // if we got a hidden attribute - assign it now.
                        const char* hidden_string = _vox_dict_get_value_as_string(&dict, "_hidden", "0");
                        if (hidden_string)
                            hidden = (hidden_string[0] == '1' ? true : false);
                    }


                    // get other properties.
                    uint32_t child_node_id, reserved_id, layer_id, num_frames;
                    _vox_file_read(fp, &child_node_id, sizeof(child_node_id));
                    _vox_file_read(fp, &reserved_id,   sizeof(reserved_id));
                    _vox_file_read(fp, &layer_id,      sizeof(layer_id));
                    _vox_file_read(fp, &num_frames,    sizeof(num_frames));
                    assert(reserved_id == UINT32_MAX && num_frames == 1); // must be these values according to the spec

                    // Parse the frame dictionary that contains:
                    //   _r : int8 ROTATION (c)
                    //   _t : int32x3 translation
                    // and extract a transform
                    ogt_vox_transform frame_transform;
                    {
                        _vox_file_read_dict(&dict, fp);
                        const char* pcRotationValue    = _vox_dict_get_value_as_string(&dict, "_r");
                        const char* pcTranslationValue = _vox_dict_get_value_as_string(&dict, "_t");
                        frame_transform = _vox_make_transform_from_dict_strings(pcRotationValue, pcTranslationValue);
                    }
                    // setup the transform node.
                    {
                        nodes.grow_to_fit_index(node_id);
                        _vox_scene_node_* transform_node = &nodes[node_id];
                        assert(transform_node);
                        transform_node->node_type = k_nodetype_transform;
                        transform_node->u.transform.child_node_id = child_node_id;
                        transform_node->u.transform.layer_id      = layer_id;
                        transform_node->u.transform.transform     = frame_transform;
                        transform_node->u.transform.hidden        = hidden;
                        // assign the name
                        _vox_strcpy_static(transform_node->u.transform.name, node_name);
                    }
                    break;
                }
                case CHUNK_ID_nGRP:
                {
                    uint32_t node_id;
                    _vox_file_read(fp, &node_id, sizeof(node_id));

                    // parse the node dictionary - data is unused.
                    _vox_file_read_dict(&dict, fp);

                    // setup the group node 
                    nodes.grow_to_fit_index(node_id);
                    _vox_scene_node_* group_node = &nodes[node_id];
                    group_node->node_type = k_nodetype_group;
                    group_node->u.group.first_child_node_id_index = 0;
                    group_node->u.group.num_child_nodes           = 0;

                    // setup all child scene nodes to point back to this node.
                    uint32_t num_child_nodes = 0;
                    _vox_file_read(fp, &num_child_nodes, sizeof(num_child_nodes));

                    // allocate space for all the child node IDs
                    if (num_child_nodes) {
                        size_t prior_size = child_ids.size();
                        assert(prior_size > 0); // should be guaranteed by the sentinel we reserved at the very beginning.
                        child_ids.resize(prior_size + num_child_nodes);
                        _vox_file_read(fp, &child_ids[prior_size], sizeof(uint32_t) * num_child_nodes);
                        group_node->u.group.first_child_node_id_index = (uint32_t)prior_size;
                        group_node->u.group.num_child_nodes = num_child_nodes;
                    }
                    break;
                }
                case CHUNK_ID_nSHP:
                {
                    uint32_t node_id;
                    _vox_file_read(fp, &node_id, sizeof(node_id));

                    // setup the shape node 
                    nodes.grow_to_fit_index(node_id);
                    _vox_scene_node_* shape_node = &nodes[node_id];
                    shape_node->node_type = k_nodetype_shape;
                    shape_node->u.shape.model_id = UINT32_MAX;

                    // parse the node dictionary - data is unused.
                    _vox_file_read_dict(&dict, fp);

                    uint32_t num_models = 0;
                    _vox_file_read(fp, &num_models, sizeof(num_models));
                    assert(num_models == 1); // must be 1 according to the spec.

                    // assign instances
                    _vox_file_read(fp, &shape_node->u.shape.model_id, sizeof(uint32_t));
                    assert(shape_node->u.shape.model_id < model_ptrs.size());

                    // parse the model dictionary - data is unsued.
                    _vox_file_read_dict(&dict, fp);
                    break;
                }
                case CHUNK_ID_IMAP:
                {
                    assert(chunk_size == 256);
                    _vox_file_read(fp, index_map, 256);
                    found_index_map_chunk = true;
                    break;
                }
                case CHUNK_ID_LAYR:
                {
                    int32_t layer_id = 0;
                    int32_t reserved_id = 0;
                    _vox_file_read(fp, &layer_id, sizeof(layer_id));
                    _vox_file_read_dict(&dict, fp);
                    _vox_file_read(fp, &reserved_id, sizeof(reserved_id));
                    assert(reserved_id == -1);

                    layers.grow_to_fit_index(layer_id);
                    layers[layer_id].name = NULL;
                    layers[layer_id].hidden = false;

                    // if we got a layer name from the LAYR dictionary, allocate space in string_data for it and keep track of the index 
                    // within string data. This will be patched to a real pointer at the very end.
                    const char* name_string = _vox_dict_get_value_as_string(&dict, "_name", NULL);
                    if (name_string) {
                        layers[layer_id].name = (const char*)(string_data.size());
                        size_t name_size = _vox_strlen(name_string) + 1;       // +1 for terminator
                        string_data.push_back_many(name_string, name_size);
                    }
                    // if we got a hidden attribute - assign it now.
                    const char* hidden_string = _vox_dict_get_value_as_string(&dict, "_hidden", "0");
                    if (hidden_string)
                        layers[layer_id].hidden = (hidden_string[0] == '1' ? true : false);
                    break;
                }
                // we don't handle MATL/MATT or any other chunks for now, so we just skip the chunk payload.
                case CHUNK_ID_MATL:
                case CHUNK_ID_MATT:
                default:
                {
                    _vox_file_seek_forwards(fp, chunk_size);
                    break;
                }
            } // end switch
        }

        // ok, now that we've parsed all scene nodes - walk the scene hierarchy, and generate instances
        // we can't do this while parsing chunks unfortunately because some chunks reference chunks
        // that are later in the file than them.
        if (nodes.size()) {
            ogt_vox_transform transform = _vox_transform_identity();
            generate_instances_for_node(nodes, 0, child_ids, 0, _vox_transform_identity(), model_ptrs, NULL, false, instances, string_data);
        }
        else if (model_ptrs.size() == 1) {
            // add a single instance
            ogt_vox_instance new_instance;
            new_instance.model_index  = 0;
            new_instance.transform   = _vox_transform_identity();
            new_instance.layer_index = 0;
            new_instance.name        = 0;
            new_instance.hidden      = false;
            instances.push_back(new_instance);
        }

        // if we didn't get a layer chunk -- just create a default layer.
        if (layers.size() == 0) {
            // go through all instances and ensure they are only mapped to layer 0
            for (uint32_t i = 0; i < instances.size(); i++)
                instances[i].layer_index = 0;
            // add a single layer
            ogt_vox_layer new_layer;
            new_layer.hidden = false;
            new_layer.name   = NULL;
            layers.push_back(new_layer);
        }

        // To support index-level assumptions (eg. artists using top 16 colors for color/palette cycling, 
        // other ranges for emissive etc), we must ensure the order of colors that the artist sees in the 
        // magicavoxel tool matches the actual index we'll end up using here. Unfortunately, magicavoxel 
        // does an unexpected thing when remapping colors in the editor using ctrl+drag within the palette.
        // Instead of remapping all indices in all models, it just keeps track of a display index to actual 
        // palette map and uses that to show reordered colors in the palette window. This is how that
        // map works:
        //   displaycolor[k] = paletteColor[imap[k]]
        // To ensure our indices are in the same order as displayed by magicavoxel within the palette
        // window, we apply the mapping from the IMAP chunk both to the color palette and indices within each 
        // voxel model.
        if (found_index_map_chunk)
        {
            // the imap chunk maps from display index to actual index.
            // generate an inverse index map (maps from actual index to display index)
            uint8_t index_map_inverse[256];
            for (uint32_t i = 0; i < 256; i++) {
                index_map_inverse[index_map[i]] = (uint8_t)i;
            }

            // reorder colors in the palette so the palette contains colors in display order
            ogt_vox_palette old_palette = palette;
            for (uint32_t i = 0; i < 256; i++) {
                uint32_t remapped_index = (index_map[i] + 255) & 0xFF;
                palette.color[i] = old_palette.color[remapped_index];
            }

            // ensure that all models are remapped so they are using display order palette indices.
            for (uint32_t i = 0; i < model_ptrs.size(); i++) {
                ogt_vox_model* model = model_ptrs[i];
                if (model) {
                    uint32_t num_voxels = model->size_x * model->size_y * model->size_z;
                    uint8_t* voxels = (uint8_t*)&model[1];
                    for (uint32_t j = 0; j < num_voxels; j++)
                        voxels[j] = 1 + index_map_inverse[voxels[j]];
                }
            }
        }

        // rotate the scene palette now so voxel indices can just map straight into the palette
        {
            ogt_vox_rgba last_color = palette.color[255];
            for (uint32_t i = 255; i > 0; i--)
                palette.color[i] = palette.color[i - 1];
            palette.color[0] = last_color;
            palette.color[0].a = 0;  // alpha is zero for the 0th color as that color index represents a transparent voxel.
        }

        // check for models that are identical by doing a pair-wise compare. If we find identical
        // models, we'll end up with NULL gaps in the model_ptrs array, but instances will have 
        // been remapped to keep the earlier model.
        for (uint32_t i = 0; i < model_ptrs.size(); i++) {
            if (!model_ptrs[i])
                continue;
            for (uint32_t j = i+1; j < model_ptrs.size(); j++) {
                if (!model_ptrs[j] || !_vox_models_are_equal(model_ptrs[i], model_ptrs[j]))
                    continue;
                // model i and model j are the same, so free model j and keep model i.
                _vox_free(model_ptrs[j]);
                model_ptrs[j] = NULL;
                // remap all instances that were referring to j to now refer to i.
                for (uint32_t k = 0; k < instances.size(); k++)
                    if (instances[k].model_index == j)
                        instances[k].model_index = i;
            }
        }

        // sometimes a model can be created which has no solid voxels within just due to the
        // authoring flow within magicavoxel. We have already have prevented creation of 
        // instances that refer to empty models, but here we want to compact the model_ptrs
        // array such that it contains no more NULL models. This also requires we remap the
        // indices for instances so they continue to refer to their correct models.
        {
            // first, check to see if we find any empty model. No need to do work otherwise.
            bool found_empty_model = false;
            for (uint32_t i = 0; i < model_ptrs.size() && !found_empty_model; i++) {
                if (model_ptrs[i] == NULL)
                    found_empty_model = true;
            }
            if (found_empty_model) {
                // build a remap table for all instances and simultaneously compact the model_ptrs array.
                uint32_t* model_remap = (uint32_t*)_vox_malloc(model_ptrs.size() * sizeof(uint32_t));
                uint32_t num_output_models = 0;
                for (uint32_t i = 0; i < model_ptrs.size(); i++) {
                    if (model_ptrs[i] != NULL) {
                        model_ptrs[num_output_models] = model_ptrs[i];
                        model_remap[i] = num_output_models;
                        num_output_models++;
                    }
                    else {
                        model_remap[i] = UINT32_MAX;
                    }
                }
                model_ptrs.resize(num_output_models);

                // remap all instances to point to the compacted model index
                for (uint32_t i = 0; i < instances.size(); i++) {
                    uint32_t new_model_index = model_remap[instances[i].model_index];
                    assert(new_model_index != UINT32_MAX);   // we should have suppressed instances already that point to NULL models.
                    instances[i].model_index = new_model_index;
                }

                // free remap table
                _vox_free(model_remap);
                model_remap = NULL;
            }
        }

        // finally, construct the output scene..
        size_t scene_size = sizeof(ogt_vox_scene) + string_data.size();
        ogt_vox_scene* scene = (ogt_vox_scene*)_vox_calloc(scene_size);
        {
            // copy name data into the scene
            char* scene_string_data = (char*)&scene[1];
            memcpy(scene_string_data, &string_data[0], sizeof(char) * string_data.size());

            // copy instances over to scene, and sort them so that instances with the same model are contiguous.
            size_t num_scene_instances = instances.size();
            ogt_vox_instance* scene_instances = (ogt_vox_instance*)_vox_malloc(sizeof(ogt_vox_instance) * num_scene_instances);
            if (num_scene_instances) {
                memcpy(scene_instances, &instances[0], sizeof(ogt_vox_instance) * num_scene_instances);
                qsort(scene_instances, num_scene_instances, sizeof(ogt_vox_instance), _vox_ordered_compare_instance);
            }
            scene->instances = scene_instances;
            scene->num_instances = (uint32_t)instances.size();

            // copy model pointers over to the scene,
            size_t num_scene_models = model_ptrs.size();
            ogt_vox_model** scene_models = (ogt_vox_model * *)_vox_malloc(sizeof(ogt_vox_model*) * num_scene_models);
            memcpy(scene_models, &model_ptrs[0], sizeof(ogt_vox_model*) * num_scene_models);
            scene->models     = (const ogt_vox_model **)scene_models;
            scene->num_models = (uint32_t)num_scene_models;

            // copy layer pointers over to the scene
            size_t num_scene_layers = layers.size();
            ogt_vox_layer* scene_layers = (ogt_vox_layer*)_vox_malloc(sizeof(ogt_vox_layer) * num_scene_layers);
            memcpy(scene_layers, &layers[0], sizeof(ogt_vox_layer) * num_scene_layers);
            scene->layers = scene_layers;
            scene->num_layers = (uint32_t)num_scene_layers;

            // now patch up instance name pointers to point into the scene string area
            for (uint32_t i = 0; i < num_scene_instances; i++)
                if (scene_instances[i].name)
                    scene_instances[i].name = scene_string_data + (size_t)scene_instances[i].name;

            // now patch up layer name pointers to point into the scene string area
            for (uint32_t i = 0; i < num_scene_layers; i++)
                if (scene_layers[i].name)
                    scene_layers[i].name = scene_string_data + (size_t)scene_layers[i].name;

            // copy the palette.
            scene->palette = palette;
        }
        return scene;
    }

    void ogt_vox_destroy_scene(const ogt_vox_scene * _scene) {
        ogt_vox_scene* scene = const_cast<ogt_vox_scene*>(_scene);
        // free models from model array
        for (uint32_t i = 0; i < scene->num_models; i++)
            _vox_free((void*)scene->models[i]);
        // free model array itself
        if (scene->models) {
            _vox_free(scene->models);
            scene->models = NULL;
        }
        // free instance array
        if (scene->instances) {
            _vox_free(const_cast<ogt_vox_instance*>(scene->instances));
            scene->instances = NULL;
        }
        // free layer array
        if (scene->layers) {
            _vox_free(const_cast<ogt_vox_layer*>(scene->layers));
            scene->layers = NULL;
        }
        // finally, free the scene.
        _vox_free(scene);
    }

    // the vector should be a unit vector aligned along one of the cardinal directions exactly. eg. (1,0,0) or (0, 0, -1)
       // this function returns the non-zero column index in out_index and the returns whether that entry is negative.
    static bool _vox_get_vec3_rotation_bits(const vec3& vec, uint32_t& out_index) {
        const float* f = &vec.x;
        out_index = 3;
        bool is_negative = false;
        for (uint32_t i = 0; i < 3; i++) {
            if (f[i] == 1.0f || f[i] == -1.0f) {
                out_index = i;
                is_negative = f[i] < 0.0f ? true : false;
            }
            else {
                assert(f[i] == 0.0f);   // must be zero 
            }
        }
        assert(out_index != 3); // if you hit this, you probably have all zeroes in the vector!
        return is_negative;
    }

    static uint8_t _vox_make_packed_rotation_from_transform(const ogt_vox_transform * transform) {
        // magicavoxel stores rows, and we have columns, so we do the swizzle here into rows
        vec3 row0 = vec3_make(transform->m00, transform->m10, transform->m20);
        vec3 row1 = vec3_make(transform->m01, transform->m11, transform->m21);
        vec3 row2 = vec3_make(transform->m02, transform->m12, transform->m22);
        uint32_t row0_index = 3, row1_index = 3, row2_index = 3;
        bool row0_negative = _vox_get_vec3_rotation_bits(row0, row0_index);
        bool row1_negative = _vox_get_vec3_rotation_bits(row1, row1_index);
        bool row2_negative = _vox_get_vec3_rotation_bits(row2, row2_index);
        const uint32_t row_mask = (1 << row0_index) | (1 << row1_index) | (1 << row2_index);
        assert(((1 << row0_index) | (1 << row1_index) | (1 << row2_index)) == 7); // check that rows are orthogonal. There must be a non-zero entry in column 0, 1 and 2 across these 3 rows.
        return (row0_index) | (row1_index << 2) | (row0_negative ? 1 << 4 : 0) | (row1_negative ? 1 << 5 : 0) | (row2_negative ? 1 << 6 : 0);
    }

    struct _vox_file_writeable {
        _vox_array<uint8_t> data;
    };

    static void _vox_file_writeable_init(_vox_file_writeable* fp) {
        fp->data.reserve(1024);
    }
    static void _vox_file_write(_vox_file_writeable* fp, const void* data, uint32_t data_size) {
        fp->data.push_back_many((const uint8_t*)data, data_size);
    }
    static void _vox_file_write_uint32(_vox_file_writeable* fp, uint32_t data) {
        fp->data.push_back_many((const uint8_t*)&data, sizeof(uint32_t));
    }
    static void _vox_file_write_uint8(_vox_file_writeable* fp, uint8_t data) {
        fp->data.push_back_many((const uint8_t*)&data, sizeof(uint8_t));
    }
    static uint32_t _vox_file_get_offset(const _vox_file_writeable* fp) {
        return (uint32_t)fp->data.count;
    }
    static uint8_t* _vox_file_get_data(_vox_file_writeable* fp) {
        return &fp->data[0];
    }
    static void _vox_file_write_dict_key_value(_vox_file_writeable* fp, const char* key, const char* value) {
        if (key == NULL || value == NULL)
            return;
        uint32_t key_len   = (uint32_t)_vox_strlen(key);
        uint32_t value_len = (uint32_t)_vox_strlen(value);
        _vox_file_write_uint32(fp, key_len);
        _vox_file_write(fp, key, key_len);
        _vox_file_write_uint32(fp, value_len);
        _vox_file_write(fp, value, value_len);
    }

    static uint32_t _vox_dict_key_value_size(const char* key, const char* value) {
        if (key == NULL || value == NULL)
            return 0;
        size_t size = sizeof(uint32_t) + _vox_strlen(key) + sizeof(uint32_t) + _vox_strlen(value);
        return (uint32_t)size;
    }

    static void _vox_file_write_chunk_nTRN(_vox_file_writeable* fp, uint32_t node_id, uint32_t child_node_id, const char* name, bool hidden, const ogt_vox_transform* transform, uint32_t layer_id)
    {
        // obtain dictionary string pointers
        const char* hidden_string = hidden ? "1" : NULL;
        const char* t_string = NULL;
        const char* r_string = NULL;
        char t_string_buf[64];
        char r_string_buf[64];
        t_string_buf[0] = 0;
        r_string_buf[0] = 0;
        if (transform != NULL) {
            uint8_t packed_rotation_bits = _vox_make_packed_rotation_from_transform(transform);
            _vox_sprintf(t_string_buf, sizeof(t_string_buf), "%i %i %i", (int32_t)transform->m30, (int32_t)transform->m31, (int32_t)transform->m32);
            _vox_sprintf(r_string_buf, sizeof(r_string_buf), "%u", packed_rotation_bits);
            t_string = t_string_buf;
            r_string = r_string_buf;
        }

        uint32_t node_dict_size =
            sizeof(uint32_t) + // num key value pairs
            _vox_dict_key_value_size("_name",   name) +        
            _vox_dict_key_value_size("_hidden", hidden_string);

        uint32_t frame_dict_size =
            sizeof(uint32_t) + // num key value pairs
            _vox_dict_key_value_size("_t", t_string) +
            _vox_dict_key_value_size("_r", r_string);

        uint32_t chunk_size_ntrn =
            sizeof(uint32_t) +   // node_id
            node_dict_size +     // node dictionary
            4 * sizeof(uint32_t) + // middle section - 4 uint32s
            frame_dict_size;

        // write the nTRN header
        _vox_file_write_uint32(fp, CHUNK_ID_nTRN);
        _vox_file_write_uint32(fp, chunk_size_ntrn);
        _vox_file_write_uint32(fp, 0);

        // write the nTRN payload
        _vox_file_write_uint32(fp, node_id);

        // write the node dictionary
        uint32_t node_dict_keyvalue_count = (name ? 1 : 0) + (hidden_string ? 1 : 0);
        _vox_file_write_uint32(fp, node_dict_keyvalue_count);  // num key values
        _vox_file_write_dict_key_value(fp, "_name", name);
        _vox_file_write_dict_key_value(fp, "_hidden", hidden_string);

        // get other properties.
        _vox_file_write_uint32(fp, child_node_id);
        _vox_file_write_uint32(fp, UINT32_MAX); // reserved_id must have all bits set.
        _vox_file_write_uint32(fp, layer_id);
        _vox_file_write_uint32(fp, 1);          // num_frames must be 1

        // write the frame dictionary
        _vox_file_write_uint32(fp, (r_string ? 1 : 0) + (t_string ? 1 : 0));  // num key values
        _vox_file_write_dict_key_value(fp, "_r", r_string);
        _vox_file_write_dict_key_value(fp, "_t", t_string);
    }

    // saves the scene out to a buffer that when saved as a .vox file can be loaded with magicavoxel.
    uint8_t* ogt_vox_write_scene(const ogt_vox_scene* scene, uint32_t& buffer_size) {
        _vox_file_writeable file;
        _vox_file_writeable_init(&file);
        _vox_file_writeable* fp = &file;

        // write file header and file version
        _vox_file_write_uint32(fp, CHUNK_ID_VOX_);
        _vox_file_write_uint32(fp, 150);

        // read the fields common to all chunks
        uint32_t chunk_id = 0;
        uint32_t chunk_size = 0;
        uint32_t chunk_child_size = 0;

        // write the main chunk 
        _vox_file_write_uint32(fp, CHUNK_ID_MAIN);
        _vox_file_write_uint32(fp, 0);
        _vox_file_write_uint32(fp, 0);  // this main_chunk_child_size will get patched up once everything is written.

        // we need to know how to patch up the main chunk size after we've written everything
        const uint32_t offset_post_main_chunk = _vox_file_get_offset(fp);

        // write out all model chunks
        for (uint32_t i = 0; i < scene->num_models; i++) {
            const ogt_vox_model* model = scene->models[i];
            assert(model->size_x <= 126 && model->size_y <= 126 && model->size_z <= 126);
            // count the number of solid voxels in the grid
            uint32_t num_voxels_in_grid = model->size_x * model->size_y * model->size_z;
            uint32_t num_solid_voxels = 0;
            for (uint32_t voxel_index = 0; voxel_index < num_voxels_in_grid; voxel_index++)
                if (model->voxel_data[voxel_index] != 0)
                    num_solid_voxels++;
            uint32_t chunk_size_xyzi = sizeof(uint32_t) + 4 * num_solid_voxels;

            // write the SIZE chunk header 
            _vox_file_write_uint32(fp, CHUNK_ID_SIZE);
            _vox_file_write_uint32(fp, 12);
            _vox_file_write_uint32(fp, 0);

            // write the SIZE chunk payload
            _vox_file_write_uint32(fp, model->size_x);
            _vox_file_write_uint32(fp, model->size_y);
            _vox_file_write_uint32(fp, model->size_z);

            // write the XYZI chunk header
            _vox_file_write_uint32(fp, CHUNK_ID_XYZI);
            _vox_file_write_uint32(fp, chunk_size_xyzi);
            _vox_file_write_uint32(fp, 0);

            // write out XYZI chunk payload
            _vox_file_write_uint32(fp, num_solid_voxels);
            uint32_t voxel_index = 0;
            for (uint32_t z = 0; z < model->size_z; z++) {
                for (uint32_t y = 0; y < model->size_y; y++) {
                    for (uint32_t x = 0; x < model->size_x; x++, voxel_index++) {
                        uint8_t color_index = model->voxel_data[voxel_index];
                        if (color_index != 0) {
                            _vox_file_write_uint8(fp, (uint8_t)x);
                            _vox_file_write_uint8(fp, (uint8_t)y);
                            _vox_file_write_uint8(fp, (uint8_t)z);
                            _vox_file_write_uint8(fp, color_index);
                        }
                    }
                }
            }
        }

        // define our node_id ranges.
        const uint32_t first_root_transform_node_id     = 0;
        const uint32_t first_group_node_id              = 1;
        const uint32_t first_shape_node_id              = 2;
        const uint32_t first_instance_transform_node_id = first_shape_node_id + scene->num_models;

        // write node 0 nTRN (root transform) 
        _vox_file_write_chunk_nTRN(fp, first_root_transform_node_id, first_group_node_id, NULL, false, NULL, UINT32_MAX);

        // write node 1 nGRP (root group that references all instance nTRN nodes as children)
        {
            uint32_t num_child_nodes = scene->num_instances;
            uint32_t chunk_size_ngrp =
                sizeof(uint32_t) +                   // node_id
                sizeof(uint32_t) +                   // num keyvalue pairs in node dictionary
                sizeof(uint32_t) +                   // num_child_nodes
                sizeof(uint32_t) * num_child_nodes;
            // write the nGRP header
            _vox_file_write_uint32(fp, CHUNK_ID_nGRP);
            _vox_file_write_uint32(fp, chunk_size_ngrp);
            _vox_file_write_uint32(fp, 0);
            // write the nGRP payload
            _vox_file_write_uint32(fp, first_group_node_id);  // node_id
            _vox_file_write_uint32(fp, 0);                    // num keyvalue pairs in node dictionary (empty!)
            _vox_file_write_uint32(fp, num_child_nodes);
            for (uint32_t i = 0; i < num_child_nodes; i++)
                _vox_file_write_uint32(fp, first_instance_transform_node_id + i);
        }

        // write out an nSHP chunk for each of the models
        for (uint32_t i = 0; i < scene->num_models; i++) {
            // compute the size of the nSHP chunk
            uint32_t chunk_size_nshp =
                sizeof(uint32_t) +      // node_id
                sizeof(uint32_t) +      // num keyvalue pairs in node dictionary
                sizeof(uint32_t) +      // num_models
                sizeof(uint32_t) +      // model_id
                sizeof(uint32_t);       // num keyvalue pairs in model dictionary
            // write the nSHP chunk header
            _vox_file_write_uint32(fp, CHUNK_ID_nSHP);
            _vox_file_write_uint32(fp, chunk_size_nshp);
            _vox_file_write_uint32(fp, 0);
            // write the nSHP chunk payload
            _vox_file_write_uint32(fp, first_shape_node_id + i);    // node_id
            _vox_file_write_uint32(fp, 0);                          // num keyvalue pairs in node dictionary
            _vox_file_write_uint32(fp, 1);                          // num_models must be 1
            _vox_file_write_uint32(fp, i);                          // model_id
            _vox_file_write_uint32(fp, 0);                          // num keyvalue pairs in model dictionary
        }
        // write out an nTRN chunk for all instances - and make them point to the relevant nSHP chunk
        for (uint32_t i = 0; i < scene->num_instances; i++) {
            const ogt_vox_instance* instance = &scene->instances[i];
            uint32_t node_id       = first_instance_transform_node_id + i;
            uint32_t child_node_id = first_shape_node_id + instance->model_index;
            _vox_file_write_chunk_nTRN(fp, node_id, child_node_id, instance->name, instance->hidden, &instance->transform, instance->layer_index);
        }

        // write out RGBA chunk for the palette
        {
            // .vox stores palette rotated by 1 color index, so do that now.
            ogt_vox_palette rotated_palette;
            for (uint32_t i = 0; i < 256; i++)
                rotated_palette.color[i] = scene->palette.color[(i + 1) & 255];

            // write the palette chunk header
            _vox_file_write_uint32(fp, CHUNK_ID_RGBA);
            _vox_file_write_uint32(fp, sizeof(ogt_vox_palette));
            _vox_file_write_uint32(fp, 0);
            // write the palette chunk payload
            _vox_file_write(fp, &rotated_palette, sizeof(ogt_vox_palette));
        }

        // write all layer chunks out.
        for (uint32_t i = 0; i < scene->num_layers; i++) {
            const char* layer_name_string = scene->layers[i].name;
            const char* hidden_string = scene->layers[i].hidden ? "1" : NULL;
            uint32_t layer_chunk_size =
                sizeof(int32_t) +   // layer_id
                sizeof(uint32_t) +  // num key value pairs
                _vox_dict_key_value_size("_name", layer_name_string) +
                _vox_dict_key_value_size("_hidden", hidden_string) +
                sizeof(int32_t);    // reserved id, must be -1
            uint32_t layer_dict_keyvalue_count = (layer_name_string ? 1 : 0) + (hidden_string ? 1 : 0);
            // write the layer chunk header
            _vox_file_write_uint32(fp, CHUNK_ID_LAYR);
            _vox_file_write_uint32(fp, layer_chunk_size);
            _vox_file_write_uint32(fp, 0);
            // write the layer chunk payload
            _vox_file_write_uint32(fp, i);                          // layer_id
            _vox_file_write_uint32(fp, layer_dict_keyvalue_count);  // num keyvalue pairs in layer dictionary 
            _vox_file_write_dict_key_value(fp, "_name",   layer_name_string);
            _vox_file_write_dict_key_value(fp, "_hidden", hidden_string);
            _vox_file_write_uint32(fp, UINT32_MAX);                 // reserved id
        }
        
        // we deliberately don't free the fp->data field, just pass the buffer pointer and size out to the caller
        buffer_size = (uint32_t)fp->data.count;
        uint8_t* buffer_data = _vox_file_get_data(fp);
        // we deliberately clear this pointer so it doesn't get auto-freed on exiting. The caller will own the memory hereafter.
        fp->data.data = NULL;

        // patch up the main chunk's child chunk size now that we've written everything we're going to write.
        {
            uint32_t* main_chunk_child_size = (uint32_t*)& buffer_data[offset_post_main_chunk - sizeof(uint32_t)];
            *main_chunk_child_size = buffer_size - offset_post_main_chunk;
        }

        return buffer_data;
    }
    
    void ogt_vox_free(void* mem)
    {
        _vox_free(mem);
    }
#endif // #ifdef OGT_VOX_IMPLEMENTATION

/* -------------------------------------------------------------------------------------------------------------------------------------------------

    MIT License

    Copyright (c) 2019 Justin Paver

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.

------------------------------------------------------------------------------------------------------------------------------------------------- */
