#include "tests.h"

TESTS_GLOBALS();

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

  ADD_TEST(testGroups);
  ADD_TEST(testMetaChunk);

  TESTS_SHUTDOWN();
}
