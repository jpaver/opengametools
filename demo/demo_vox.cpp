/*
    demo_vox - MIT license - Justin Paver, Oct 2019

    A demonstration program to show you how to use the MagicaVoxel scene reader and
    writer from the open game tools project: https://github.com/jpaver/opengametools.

    Please see the MIT license information at the end of this file, and please consider 
    sharing any improvements you make.
*/

#define OGT_VOX_IMPLEMENTATION
#include "..\src\ogt_vox.h"
#include <io.h>


// a helper function to load a magica voxel scene given a filename.
const ogt_vox_scene* load_vox_scene(const char* pcFilename)
{
	// open the file
#if defined(_MSC_VER) && _MSC_VER >= 1400
    FILE * fp;
    if (0 != fopen_s(&fp, pcFilename, "rb"))
        fp = 0;
#else
    FILE * fp = fopen(pcFilename, "rb");
#endif
    if (!fp)
        return NULL;

	// load the file into a memory buffer
    uint32_t buffersize = _filelength(_fileno(fp));
    uint8_t* buffer = new uint8_t[buffersize];
    fread(buffer, buffersize, 1, fp);
    fclose(fp);

    // construct the scene from the buffer
    const ogt_vox_scene * scene = ogt_vox_read_scene(buffer, buffersize);

    // the buffer can be safely deleted once the scene is instantiated.
    delete[] buffer;

    return scene;
}


// a helper function to save a magica voxel scene to disk.
void save_vox_scene(const char* pcFilename, const ogt_vox_scene* scene) 
{
    // save the scene back out. 
    uint32_t buffersize = 0;
    uint8_t* buffer = ogt_vox_write_scene(scene, buffersize);
    if (!buffer)
        return;

    // open the file for write
#if defined(_MSC_VER) && _MSC_VER >= 1400
    FILE * fp;
    if (0 != fopen_s(&fp, pcFilename, "wb"))
        fp = 0;
#else
    FILE * fp = fopen(pcFilename, "wb");
#endif
    if (!fp) {
        ogt_vox_free(buffer);
        return;
    }

    fwrite(buffer, buffersize, 1, fp);
    fclose(fp);
    ogt_vox_free(buffer);
}

// this example just counts the number of solid voxels in this model, but an importer 
// would probably do something like convert the model into a triangle mesh.
uint32_t count_solid_voxels_in_model(const ogt_vox_model* model)
{
    uint32_t solid_voxel_count = 0;
    uint32_t voxel_index = 0;
    for (uint32_t z = 0; z < model->size_z; z++) {
        for (uint32_t y = 0; y < model->size_y; y++) {
            for (uint32_t x = 0; x < model->size_x; x++, voxel_index++) {
                // if color index == 0, this voxel is empty, otherwise it is solid.
                uint32_t color_index = model->voxel_data[voxel_index];
                bool is_voxel_solid = (color_index != 0);
                // add to our accumulator
                solid_voxel_count += (is_voxel_solid ? 1 : 0);
            }
        }
    }
    return solid_voxel_count;
}

int main(int argc, char** argv)
{
    const ogt_vox_scene* scene = load_vox_scene("test_tileset.vox");
    if (scene)
    {
        printf("#layers: %u\n", scene->num_layers);
        for (uint32_t layer_index = 0; layer_index < scene->num_layers; layer_index++)
        {
            const ogt_vox_layer* layer = &scene->layers[layer_index];
            printf("layer[%u,name=%s] is %s\n",
                layer_index,
                layer->name ? layer->name : "",
                layer->hidden ? "hidden" : "shown");
        }
        // iterate over all instances - and print basic information about the instance and the model that it references
        printf("# instances: %u\n", scene->num_instances);
        for (uint32_t instance_index = 0; instance_index < scene->num_instances; instance_index++)
        {
            const ogt_vox_instance* instance = &scene->instances[instance_index];
            const ogt_vox_model* model = scene->models[instance->model_index];
            
            const char* layer_name =
                instance->layer_index == UINT32_MAX ? "(no layer)":
                scene->layers[instance->layer_index].name ? scene->layers[instance->layer_index].name : 
                "";

            printf("instance[%u,name=%s] at position (%.0f,%.0f,%.0f) uses model %u and is in layer[%u, name='%s'] and is %s\n",
                instance_index,
                instance->name ? instance->name : "",
                instance->transform.m30, instance->transform.m31, instance->transform.m32, // translation components of the instance
                instance->model_index,
                instance->layer_index,
                layer_name,
                instance->hidden ? "hidden" : "shown");
        }
        // iterate over all models and print basic information about the model.
        printf("# models: %u\n", scene->num_models);
        for (uint32_t model_index = 0; model_index < scene->num_models; model_index++)
        {
            const ogt_vox_model* model = scene->models[model_index];

            uint32_t solid_voxel_count = count_solid_voxels_in_model(model);
            uint32_t total_voxel_count = model->size_x * model->size_y * model->size_z;

            printf(" model[%u] has dimension %ux%ux%u, with %u solid voxels of the total %u voxels (hash=%u)!\n",
                model_index,
                model->size_x, model->size_y, model->size_z,
                solid_voxel_count,
                total_voxel_count,
                model->voxel_hash);
        }

        save_vox_scene("test_tileset_SAVED.vox", scene); 

        ogt_vox_destroy_scene(scene);
    }
    return 0;
}


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