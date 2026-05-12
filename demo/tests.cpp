#include "tests.h"

TESTS_GLOBALS();

static ogt_vox_sun *ensureSun(ogt_vox_scene *scene) {
  if (scene->sun == nullptr) {
    scene->sun = (ogt_vox_sun *)_vox_malloc(sizeof(ogt_vox_sun));
    memset(scene->sun, 0, sizeof(*scene->sun));
  }
  return scene->sun;
}

static ogt_vox_atmosphere *ensureAtmosphere(ogt_vox_scene *scene) {
  if (scene->atmosphere == nullptr) {
    scene->atmosphere =
        (ogt_vox_atmosphere *)_vox_malloc(sizeof(ogt_vox_atmosphere));
    memset(scene->atmosphere, 0, sizeof(*scene->atmosphere));
  }
  return scene->atmosphere;
}

static ogt_vox_fog *ensureFog(ogt_vox_scene *scene) {
  if (scene->fog == nullptr) {
    scene->fog = (ogt_vox_fog *)_vox_malloc(sizeof(ogt_vox_fog));
    memset(scene->fog, 0, sizeof(*scene->fog));
  }
  return scene->fog;
}

static ogt_vox_post_process *ensurePostProcess(ogt_vox_scene *scene) {
  if (scene->post_process == nullptr) {
    scene->post_process =
        (ogt_vox_post_process *)_vox_malloc(sizeof(ogt_vox_post_process));
    memset(scene->post_process, 0, sizeof(*scene->post_process));
  }
  return scene->post_process;
}

static ogt_vox_display *ensureDisplay(ogt_vox_scene *scene) {
  if (scene->display == nullptr) {
    scene->display = (ogt_vox_display *)_vox_malloc(sizeof(ogt_vox_display));
    memset(scene->display, 0, sizeof(*scene->display));
  }
  return scene->display;
}

static void expectRgba(const ogt_vox_rgba &expected, const ogt_vox_rgba &actual) {
  EXPECT_EQ_UINT(expected.r, actual.r);
  EXPECT_EQ_UINT(expected.g, actual.g);
  EXPECT_EQ_UINT(expected.b, actual.b);
  EXPECT_EQ_UINT(expected.a, actual.a);
}

static void testLoadScene(void) {
  const ogt_vox_scene *scene = load_vox_scene_with_groups("test_meta_chunk.vox");
  ASSERT_NE_NULLPTR(scene);
  EXPECT_EQ_UINT(7u, scene->anim_range_start);
  EXPECT_EQ_UINT(36u, scene->anim_range_end);
  EXPECT_EQ_UINT(200u, scene->file_version);

  EXPECT_EQ_UINT(10u, scene->num_cameras);
  ASSERT_NE_NULLPTR(scene->cameras);

  ASSERT_EQ_UINT(1u, scene->num_models);
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

  EXPECT_EQ_UINT(32u, scene->num_color_names);
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

static void testRoundtripExtendedSceneFlags(void) {
  const ogt_vox_scene *input_scene = load_vox_scene_with_groups("test_meta_chunk.vox");
  ASSERT_NE_NULLPTR(input_scene);

  ogt_vox_scene *scene = const_cast<ogt_vox_scene *>(input_scene);
  ASSERT_NE_NULLPTR(scene->cameras);
  ASSERT_NE_NULLPTR(scene->models);
  ASSERT_NE_NULLPTR(scene->groups);

  ogt_vox_cam &camera = const_cast<ogt_vox_cam *>(scene->cameras)[0];
  camera.aperture = 2.25f;
  camera.blade_num = 7u;
  camera.blade_rotation = 13.5f;

  ogt_vox_sun *sun = ensureSun(scene);
  sun->intensity = 2.75f;
  sun->area = 0.33f;
  sun->angle[0] = 17.25f;
  sun->angle[1] = 201.5f;
  sun->rgba = {12, 34, 56, 255};
  sun->disk = true;

  ogt_vox_atmosphere *atmosphere = ensureAtmosphere(scene);
  atmosphere->ray_density = 0.11f;
  atmosphere->ray_color = {20, 40, 60, 255};
  atmosphere->mie_density = 0.22f;
  atmosphere->mie_color = {80, 100, 120, 255};
  atmosphere->mie_g = 0.44f;
  atmosphere->o3_density = 0.55f;
  atmosphere->o3_color = {140, 160, 180, 255};

  ogt_vox_fog *fog = ensureFog(scene);
  fog->density = 0.66f;
  fog->color = {90, 91, 92, 255};
  fog->height = 7.5f;

  ogt_vox_post_process *post_process = ensurePostProcess(scene);
  post_process->exposure = 1.25f;
  post_process->vignette = 0.18f;
  post_process->aces = true;
  post_process->bloom_mix = 0.28f;
  post_process->bloom_scale = 0.38f;
  post_process->bloom_aspect = 0.48f;
  post_process->bloom_threshold = 0.58f;

  ogt_vox_display *display = ensureDisplay(scene);
  display->ground_color = {1, 2, 3, 255};
  display->ground_horizon = 4.25f;
  display->edge_color = {5, 6, 7, 255};
  display->edge_width = 0.75f;
  display->grid_color = {8, 9, 10, 255};
  display->grid_spacing = 11u;
  display->grid_width = 1.5f;
  display->show_ground = true;
  display->show_grid = false;
  display->show_edge = true;

  ogt_vox_matl &material = scene->materials.matl[17];
  material.content_flags =
      k_ogt_vox_matl_have_rough | k_ogt_vox_matl_have_spec |
      k_ogt_vox_matl_have_media | k_ogt_vox_matl_have_ri |
      k_ogt_vox_matl_have_plastic | k_ogt_vox_matl_have_g0 |
      k_ogt_vox_matl_have_g1 | k_ogt_vox_matl_have_gw |
      k_ogt_vox_matl_have_spec_p | k_ogt_vox_matl_have_absorb |
      k_ogt_vox_matl_have_scatter | k_ogt_vox_matl_have_sss;
  material.type = ogt_matl_type_media;
  material.media_type = ogt_media_type_sss;
  material.rough = 0.21f;
  material.spec = 0.31f;
  material.media = 0.41f;
  material.ri = 1.61f;
  material.plastic = 0.51f;
  material.g0 = -0.25f;
  material.g1 = 0.35f;
  material.gw = 0.45f;
  material.spec_p = 0.55f;
  material.absorb = 0.65f;
  material.scatter = 0.75f;
  material.sss = 0.85f;

  const char *filename = "roundtrip_scene_flags.vox";
  EXPECT_TRUE(save_vox_scene(filename, scene));

  const ogt_vox_scene *roundtrip_scene = load_vox_scene_with_groups(filename);
  ASSERT_NE_NULLPTR(roundtrip_scene);
  ASSERT_NE_NULLPTR(roundtrip_scene->cameras);
  ASSERT_NE_NULLPTR(roundtrip_scene->sun);
  ASSERT_NE_NULLPTR(roundtrip_scene->atmosphere);
  ASSERT_NE_NULLPTR(roundtrip_scene->fog);
  ASSERT_NE_NULLPTR(roundtrip_scene->post_process);
  ASSERT_NE_NULLPTR(roundtrip_scene->display);

  const ogt_vox_cam &roundtrip_camera = roundtrip_scene->cameras[0];
  EXPECT_EQ_FLOAT(2.25f, roundtrip_camera.aperture, 0.0001f);
  EXPECT_EQ_UINT(7u, roundtrip_camera.blade_num);
  EXPECT_EQ_FLOAT(13.5f, roundtrip_camera.blade_rotation, 0.0001f);

  const ogt_vox_sun &roundtrip_sun = *roundtrip_scene->sun;
  EXPECT_EQ_FLOAT(2.75f, roundtrip_sun.intensity, 0.0001f);
  EXPECT_EQ_FLOAT(0.33f, roundtrip_sun.area, 0.0001f);
  EXPECT_EQ_FLOAT(17.25f, roundtrip_sun.angle[0], 0.0001f);
  EXPECT_EQ_FLOAT(201.5f, roundtrip_sun.angle[1], 0.0001f);
  expectRgba({12, 34, 56, 255}, roundtrip_sun.rgba);
  EXPECT_TRUE(roundtrip_sun.disk);

  const ogt_vox_atmosphere &roundtrip_atmosphere = *roundtrip_scene->atmosphere;
  EXPECT_EQ_FLOAT(0.11f, roundtrip_atmosphere.ray_density, 0.0001f);
  expectRgba({20, 40, 60, 255}, roundtrip_atmosphere.ray_color);
  EXPECT_EQ_FLOAT(0.22f, roundtrip_atmosphere.mie_density, 0.0001f);
  expectRgba({80, 100, 120, 255}, roundtrip_atmosphere.mie_color);
  EXPECT_EQ_FLOAT(0.44f, roundtrip_atmosphere.mie_g, 0.0001f);
  EXPECT_EQ_FLOAT(0.55f, roundtrip_atmosphere.o3_density, 0.0001f);
  expectRgba({140, 160, 180, 255}, roundtrip_atmosphere.o3_color);

  const ogt_vox_fog &roundtrip_fog = *roundtrip_scene->fog;
  EXPECT_EQ_FLOAT(0.66f, roundtrip_fog.density, 0.0001f);
  expectRgba({90, 91, 92, 255}, roundtrip_fog.color);
  EXPECT_EQ_FLOAT(7.5f, roundtrip_fog.height, 0.0001f);

  const ogt_vox_post_process &roundtrip_post_process =
      *roundtrip_scene->post_process;
  EXPECT_EQ_FLOAT(1.25f, roundtrip_post_process.exposure, 0.0001f);
  EXPECT_EQ_FLOAT(0.18f, roundtrip_post_process.vignette, 0.0001f);
  EXPECT_TRUE(roundtrip_post_process.aces);
  EXPECT_EQ_FLOAT(0.28f, roundtrip_post_process.bloom_mix, 0.0001f);
  EXPECT_EQ_FLOAT(0.38f, roundtrip_post_process.bloom_scale, 0.0001f);
  EXPECT_EQ_FLOAT(0.48f, roundtrip_post_process.bloom_aspect, 0.0001f);
  EXPECT_EQ_FLOAT(0.58f, roundtrip_post_process.bloom_threshold, 0.0001f);

  const ogt_vox_display &roundtrip_display = *roundtrip_scene->display;
  expectRgba({1, 2, 3, 255}, roundtrip_display.ground_color);
  EXPECT_EQ_FLOAT(4.25f, roundtrip_display.ground_horizon, 0.0001f);
  expectRgba({5, 6, 7, 255}, roundtrip_display.edge_color);
  EXPECT_EQ_FLOAT(0.75f, roundtrip_display.edge_width, 0.0001f);
  expectRgba({8, 9, 10, 255}, roundtrip_display.grid_color);
  EXPECT_EQ_UINT(11u, roundtrip_display.grid_spacing);
  EXPECT_EQ_FLOAT(1.5f, roundtrip_display.grid_width, 0.0001f);
  EXPECT_TRUE(roundtrip_display.show_ground);
  EXPECT_FALSE(roundtrip_display.show_grid);
  EXPECT_TRUE(roundtrip_display.show_edge);

  const ogt_vox_matl &roundtrip_material = roundtrip_scene->materials.matl[17];
  EXPECT_EQ_UINT(material.content_flags, roundtrip_material.content_flags);
  EXPECT_EQ_INT((int)material.type, (int)roundtrip_material.type);
  EXPECT_EQ_INT((int)material.media_type, (int)roundtrip_material.media_type);
  EXPECT_EQ_FLOAT(0.21f, roundtrip_material.rough, 0.0001f);
  EXPECT_EQ_FLOAT(0.31f, roundtrip_material.spec, 0.0001f);
  EXPECT_EQ_FLOAT(0.41f, roundtrip_material.media, 0.0001f);
  EXPECT_EQ_FLOAT(1.61f, roundtrip_material.ri, 0.0001f);
  EXPECT_EQ_FLOAT(0.51f, roundtrip_material.plastic, 0.0001f);
  EXPECT_EQ_FLOAT(-0.25f, roundtrip_material.g0, 0.0001f);
  EXPECT_EQ_FLOAT(0.35f, roundtrip_material.g1, 0.0001f);
  EXPECT_EQ_FLOAT(0.45f, roundtrip_material.gw, 0.0001f);
  EXPECT_EQ_FLOAT(0.55f, roundtrip_material.spec_p, 0.0001f);
  EXPECT_EQ_FLOAT(0.65f, roundtrip_material.absorb, 0.0001f);
  EXPECT_EQ_FLOAT(0.75f, roundtrip_material.scatter, 0.0001f);
  EXPECT_EQ_FLOAT(0.85f, roundtrip_material.sss, 0.0001f);

  ogt_vox_destroy_scene(roundtrip_scene);
  ogt_vox_destroy_scene(input_scene);
  remove(filename);
}

int main(int argc, char *argv[]) {
  TESTS_INIT();

  ADD_TEST(testLoadScene);
  ADD_TEST(testGroups);
  ADD_TEST(testMetaChunk);
  ADD_TEST(testRoundtripExtendedSceneFlags);

  TESTS_SHUTDOWN();
}
