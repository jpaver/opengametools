#include "tests.h"

TESTS_GLOBALS();

static void testLoadScene(void) {
  const ogt_vox_scene *scene = load_vox_scene_with_groups("test_meta_chunk.vox");
  ASSERT_NE_NULLPTR(scene);
  EXPECT_EQ_INT(7, (int)scene->anim_range_start);
  EXPECT_EQ_INT(36, (int)scene->anim_range_end);
  EXPECT_EQ_INT(200, (int)scene->file_version);

  EXPECT_EQ_INT(10, (int)scene->num_cameras);
  ASSERT_NE_NULLPTR(scene->cameras);

  ASSERT_EQ_INT(1, (int)scene->num_models);
  ASSERT_NE_NULLPTR(scene->models);
  const ogt_vox_model *model = scene->models[0];
  EXPECT_EQ_UINT(64000u, count_solid_voxels_in_model(model));
  EXPECT_EQ_UINT(40u, model->size_x);
  EXPECT_EQ_UINT(40u, model->size_y);
  EXPECT_EQ_UINT(40u, model->size_z);

  EXPECT_EQ_UINT(1u, scene->num_instances);
  ASSERT_NE_NULLPTR(scene->instances);

  EXPECT_EQ_UINT(16u, scene->num_layers);
  ASSERT_NE_NULLPTR(scene->layers);
  const ogt_vox_layer &layer = scene->layers[0];
  EXPECT_EQ_NULLPTR(layer.name);
  EXPECT_FALSE(layer.hidden);
  EXPECT_EQ_UINT(255u, layer.color.r);
  EXPECT_EQ_UINT(204u, layer.color.g);
  EXPECT_EQ_UINT(153u, layer.color.b);
  EXPECT_EQ_UINT(255u, layer.color.a);

  EXPECT_EQ_UINT(1u, scene->num_groups);
  ASSERT_NE_NULLPTR(scene->groups);
  const ogt_vox_group &group = scene->groups[0];
  EXPECT_EQ_NULLPTR(group.name);
  EXPECT_EQ_UINT(k_invalid_group_index, group.parent_group_index);
  EXPECT_EQ_UINT(k_invalid_layer_index, group.layer_index);
  EXPECT_FALSE(group.hidden);

  EXPECT_EQ_INT(32, (int)scene->num_color_names);
  ASSERT_NE_NULLPTR(scene->color_names);
  EXPECT_EQ_STRING("NOTE", scene->color_names[0]);
}

static void testGroups(void) {
  const ogt_vox_scene *scene = load_vox_scene_with_groups("test_groups.vox");
  ASSERT_NE_NULLPTR(scene);
  ASSERT_EQ_INT(5, (int)scene->num_groups);
  ASSERT_EQ_INT(150, (int)scene->file_version);
  ASSERT_NE_NULLPTR(scene->groups);
  EXPECT_EQ_STRING("characters", scene->groups[3].name);
  EXPECT_EQ_STRING("text", scene->groups[4].name);
}

static void testMetaChunk(void) {
  const char *filename = loadsave_vox_scene("test_meta_chunk.vox");
  ASSERT_NE_NULLPTR(filename);
  const ogt_vox_scene *scene = load_vox_scene_with_groups(filename);
  ASSERT_NE_NULLPTR(scene);
  EXPECT_EQ_INT(7, (int)scene->anim_range_start);
  EXPECT_EQ_INT(36, (int)scene->anim_range_end);
  EXPECT_EQ_INT(200, (int)scene->file_version);
}

int main(int argc, char *argv[]) {
  TESTS_INIT();

  ADD_TEST(testLoadScene);
  ADD_TEST(testGroups);
  ADD_TEST(testMetaChunk);

  TESTS_SHUTDOWN();
}
