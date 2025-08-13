#ifndef TEST_SHARED_H
#define TEST_SHARED_H

#define OGT_VOX_IMPLEMENTATION
#include "../src/ogt_vox.h"

#if defined(_MSC_VER)
#include <io.h>
#endif
#include <stdio.h>

inline uint32_t count_solid_voxels_in_model(const ogt_vox_model *model) {
  uint32_t solid_voxel_count = 0;
  uint32_t voxel_index = 0;
  for (uint32_t z = 0; z < model->size_z; z++) {
    for (uint32_t y = 0; y < model->size_y; y++) {
      for (uint32_t x = 0; x < model->size_x; x++, voxel_index++) {
        uint32_t color_index = model->voxel_data[voxel_index];
        bool is_voxel_solid = (color_index != 0);
        solid_voxel_count += (is_voxel_solid ? 1 : 0);
      }
    }
  }
  return solid_voxel_count;
}

// a helper function to load a magica voxel scene given a filename.
inline const ogt_vox_scene *load_vox_scene(const char *filename,
                                           uint32_t scene_read_flags = 0) {
  if (filename == nullptr) {
    fprintf(stderr, "No filename given\n");
    return nullptr;
  }
  // open the file
#if defined(_MSC_VER) && _MSC_VER >= 1400
  FILE *fp;
  if (0 != fopen_s(&fp, filename, "rb"))
    fp = 0;
#else
  FILE *fp = fopen(filename, "rb");
#endif
  if (!fp) {
    fprintf(stderr, "Failed to load %s\n", filename);
    return nullptr;
  }

  // get the buffer size which matches the size of the file
  fseek(fp, 0, SEEK_END);
  uint32_t buffer_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  // load the file into a memory buffer
  uint8_t *buffer = new uint8_t[buffer_size];
  fread(buffer, buffer_size, 1, fp);
  fclose(fp);

  // construct the scene from the buffer
  const ogt_vox_scene *scene =
      ogt_vox_read_scene_with_flags(buffer, buffer_size, scene_read_flags);

  // the buffer can be safely deleted once the scene is instantiated.
  delete[] buffer;

  return scene;
}

inline const ogt_vox_scene *load_vox_scene_with_groups(const char *filename) {
  return load_vox_scene(filename, k_read_scene_flags_groups);
}

// a helper function to save a magica voxel scene to disk.
inline bool save_vox_scene(const char *pcFilename, const ogt_vox_scene *scene) {
  if (pcFilename == nullptr) {
    fprintf(stderr, "No filename for saving given\n");
    return false;
  }
  if (scene == nullptr) {
    fprintf(stderr, "Failed to save vox scene to %s\n", pcFilename);
    return false;
  }
  // save the scene back out.
  uint32_t buffersize = 0;
  uint8_t *buffer = ogt_vox_write_scene(scene, &buffersize);
  if (buffer == nullptr) {
    fprintf(stderr, "Failed to write vox scene to %s\n", pcFilename);
    return false;
  }

  // open the file for write
#if defined(_MSC_VER) && _MSC_VER >= 1400
  FILE *fp;
  if (0 != fopen_s(&fp, pcFilename, "wb"))
    fp = 0;
#else
  FILE *fp = fopen(pcFilename, "wb");
#endif
  if (!fp) {
    fprintf(stderr, "Failed to open %s for writing\n", pcFilename);
    ogt_vox_free(buffer);
    return false;
  }

  fwrite(buffer, buffersize, 1, fp);
  fclose(fp);
  ogt_vox_free(buffer);
  return true;
}

inline const char *loadsave_vox_scene(const char *filename) {
  static const char *target_filename = "test.vox";
  const ogt_vox_scene *scene = load_vox_scene_with_groups(filename);
  if (!save_vox_scene(target_filename, scene)) {
    return nullptr;
  }
  return target_filename;
}

#define TEST_STRINGIFY(arg) #arg

#define ADD_TEST(func)                                                         \
  prevFailed = failed;                                                         \
  errorBuf[0] = '\0';                                                          \
  printf("Testing  %-30s...", TEST_STRINGIFY(func));                           \
  (func)();                                                                    \
  if (prevFailed == failed) {                                                  \
    printf("  [success]\n");                                                   \
  } else {                                                                     \
    printf("   [failed]\n");                                                   \
    printf("%s", errorBuf);                                                    \
  }                                                                            \
  ++tests

#define ADD_DISABLED_TEST(func)                                                \
  prevFailed = failed;                                                         \
  errorBuf[0] = '\0';                                                          \
  if (!runDisabled) {                                                          \
    printf("Skipping %-30s...", TEST_STRINGIFY(func));                         \
    printf("  [skip]\n");                                                      \
  } else {                                                                     \
    printf("Testing  %-30s...", TEST_STRINGIFY(func));                         \
    (func)();                                                                  \
    if (prevFailed == failed) {                                                \
      printf("  [success]\n");                                                 \
    } else {                                                                   \
      printf("   [failed]\n");                                                 \
      printf("%s", errorBuf);                                                  \
    }                                                                          \
  }                                                                            \
  ++tests

#define TESTS_GLOBALS()                                                        \
  static int failed = 0;                                                       \
  static int tests = 0;                                                        \
  static int prevFailed = 0;                                                   \
  static char errorBuf[4096] = "";                                             \
  static int lastExpectedInt = 0;                                              \
  static unsigned int lastExpectedUInt = 0;                                    \
  static float lastExpectedFloat = 0.0f;                                       \
  static const char *lastExpectedString = nullptr;                             \
  static bool lastExpectedBool = false;                                        \
  static int runDisabled = 0;

#define TESTS_SHUTDOWN()                                                       \
  printf("\nfailed tests: %i out of %i\n", failed, tests);                     \
  if (failed != 0) {                                                           \
    return 1;                                                                  \
  }                                                                            \
  return 0

#define TESTS_INIT()                                                           \
  (void)lastExpectedInt;                                                       \
  (void)lastExpectedUInt;                                                      \
  (void)lastExpectedFloat;                                                     \
  (void)lastExpectedString;                                                    \
  (void)lastExpectedBool;                                                      \
  (void)runDisabled;                                                           \
  (void)failed;                                                                \
  (void)tests;                                                                 \
  (void)prevFailed;                                                            \
  if (argc > 1) {                                                              \
    if (!strcmp(argv[1], "--also_run_disabled_tests")) {                       \
      runDisabled = 1;                                                         \
    } else if (!strcmp(argv[1], "--help")) {                                   \
      printf("--also_run_disabled_tests : also run disabled tests");           \
      return 0;                                                                \
    }                                                                          \
  }

#define EXPECT_TRUE(actual)                                                    \
  if (lastExpectedBool = (actual),                                             \
      lastExpectedBool == false) {                                             \
    snprintf(errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf), \
             " - " TEST_STRINGIFY(                                             \
                 actual) ": expected true (line %i)\n", __LINE__);             \
    ++failed;                                                                  \
  }

#define EXPECT_FALSE(actual)                                                   \
  if (lastExpectedBool = (actual),                                             \
      lastExpectedBool == true) {                                              \
    snprintf(errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf), \
             " - " TEST_STRINGIFY(                                             \
                 actual) ": expected false (line %i)\n", __LINE__);            \
    ++failed;                                                                  \
  }

#define ASSERT_EQ_FLOAT(exp, actual)                                           \
  if (lastExpectedFloat = (actual),                                            \
      abs((exp) - lastExpectedFloat) > (epsilon)) {                            \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(actual) ": expected %f, but got %f (line %i)\n",  \
        exp, lastExpectedFloat, __LINE__);                                     \
    ++failed;                                                                  \
    return;                                                                    \
  }

#define EXPECT_EQ_FLOAT(exp, actual, epsilon)                                  \
  if (lastExpectedFloat = (actual),                                            \
      abs((exp) - lastExpectedFloat) > (epsilon)) {                            \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(actual) ": expected %f, but got %f (line %i)\n",  \
        exp, lastExpectedFloat, __LINE__);                                     \
    ++failed;                                                                  \
  }

#define ASSERT_EQ_INT(exp, actual)                                             \
  if (lastExpectedInt = (actual), (exp) != lastExpectedInt) {                  \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(actual) ": expected %i, but got %i (line %i)\n",  \
        exp, lastExpectedInt, __LINE__);                                       \
    ++failed;                                                                  \
    return;                                                                    \
  }

#define EXPECT_EQ_INT(exp, actual)                                             \
  if (lastExpectedInt = (actual), (exp) != lastExpectedInt) {                  \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(actual) ": expected %i, but got %i (line %i)\n",  \
        exp, lastExpectedInt, __LINE__);                                       \
    ++failed;                                                                  \
  }

#define ASSERT_EQ_UINT(exp, actual)                                            \
  if (lastExpectedUInt = (actual), (exp) != lastExpectedUInt) {                \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(actual) ": expected %u, but got %u (line %i)\n",  \
        exp, lastExpectedUInt, __LINE__);                                      \
    ++failed;                                                                  \
    return;                                                                    \
  }

#define EXPECT_EQ_UINT(exp, actual)                                            \
  if (lastExpectedUInt = (actual), (exp) != lastExpectedUInt) {                \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(actual) ": expected %u, but got %u (line %i)\n",  \
        exp, lastExpectedUInt, __LINE__);                                      \
    ++failed;                                                                  \
  }

#define EXPECT_BETWEEN_INT(minv, maxv, actual)                                 \
  if (lastExpectedInt = (actual),                                              \
      lastExpectedInt < (minv) || lastExpectedInt > (maxv)) {                  \
    snprintf(errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf), \
             " - " TEST_STRINGIFY(                                             \
                 actual) ": expected %i to in range of [%i:%i] (line %i)\n",   \
             lastExpectedInt, minv, maxv, __LINE__);                           \
    ++failed;                                                                  \
  }

#define EXPECT_GT_INT(exp, actual)                                             \
  if (lastExpectedInt = (actual), (exp) >= lastExpectedInt) {                  \
    snprintf(errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf), \
             " - " TEST_STRINGIFY(actual) ": expected to be greater than %i, " \
                                          "but got %i (line %i)\n",            \
             exp, lastExpectedInt, __LINE__);                                  \
    ++failed;                                                                  \
  }

#define EXPECT_GE_INT(exp, actual)                                             \
  if (lastExpectedInt = (actual), (exp) > lastExpectedInt) {                   \
    snprintf(errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf), \
             " - " TEST_STRINGIFY(actual) ": expected to be greater or equal " \
                                          "to %i, but got %i (line %i)\n",     \
             exp, lastExpectedInt, __LINE__);                                  \
    ++failed;                                                                  \
  }

#define EXPECT_LT_INT(exp, actual)                                             \
  if (lastExpectedInt = (actual), (exp) <= lastExpectedInt) {                  \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(                                                  \
            actual) ": expected to be less than %i, but got %i (line %i)\n",   \
        exp, lastExpectedInt, __LINE__);                                       \
    ++failed;                                                                  \
  }

#define EXPECT_LE_INT(exp, actual)                                             \
  if (lastExpectedInt = (actual), (exp) < lastExpectedInt) {                   \
    snprintf(errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf), \
             " - " TEST_STRINGIFY(actual) ": expected to be less or equal to " \
                                          "%i, but got %i (line %i)\n",        \
             exp, lastExpectedInt, __LINE__);                                  \
    ++failed;                                                                  \
  }

#define EXPECT_NE_NULLPTR(actual)                                              \
  if ((actual) == nullptr) {                                                   \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(actual) ": expected to be not null (line %i)\n",  \
        __LINE__);                                                             \
    ++failed;                                                                  \
  }

#define ASSERT_NE_NULLPTR(actual)                                              \
  if ((actual) == nullptr) {                                                   \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(actual) ": expected to be not null (line %i)\n",  \
        __LINE__);                                                             \
    ++failed;                                                                  \
    return;                                                                    \
  }

#define EXPECT_EQ_NULLPTR(actual)                                              \
  if ((actual) != nullptr) {                                                   \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(actual) ": expected to be null (line %i)\n",      \
        __LINE__);                                                             \
    ++failed;                                                                  \
  }

#define ASSERT_EQ_NULLPTR(actual)                                              \
  if ((actual) != nullptr) {                                                   \
    snprintf(                                                                  \
        errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf),      \
        " - " TEST_STRINGIFY(actual) ": expected to be null (line %i)\n",      \
        __LINE__);                                                             \
    ++failed;                                                                  \
    return;                                                                    \
  }

#define ASSERT_EQ_STRING(exp, actual)                                          \
  if (lastExpectedString = (actual), strcmp(exp, lastExpectedString) != 0) {   \
    snprintf(errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf), \
             " - " TEST_STRINGIFY(                                             \
                 actual) ": expected '%s', but got '%s' (line %i)\n",          \
             exp, lastExpectedString, __LINE__);                               \
    ++failed;                                                                  \
    return;                                                                    \
  }

#define EXPECT_EQ_STRING(exp, actual)                                          \
  if (lastExpectedString = (actual), strcmp(exp, lastExpectedString) != 0) {   \
    snprintf(errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf), \
             " - " TEST_STRINGIFY(                                             \
                 actual) ": expected '%s', but got '%s' (line %i)\n",          \
             exp, lastExpectedString, __LINE__);                               \
    ++failed;                                                                  \
  }

#define EXPECT_NE_STRING(exp, actual)                                          \
  if (lastExpectedString = (actual), strcmp(exp, lastExpectedString) == 0) {   \
    snprintf(errorBuf + strlen(errorBuf), sizeof(errorBuf) - strlen(errorBuf), \
             " - " TEST_STRINGIFY(                                             \
                 actual) ": expected '%s', but got '%s' (line %i)\n",          \
             exp, lastExpectedString, __LINE__);                               \
    ++failed;                                                                  \
  }

#endif
