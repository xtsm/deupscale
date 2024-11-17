#include "deupscale.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

extern "C"
bool DeupscaleFile(const char* infile, const char* outfile) {
  int x, y, n;
  auto data = stbi_load(infile, &x, &y, &n, 0);
  if (data == nullptr || x < 1 || y < 1 || n < 1) {
    std::cerr << "could not read " << infile << std::endl;
    return false;
  }

  size_t width = x;
  size_t height = y;
  if (!Deupscale(data, &width, &height, n)) {
    std::cerr << "could not deupscale" << std::endl;
    stbi_image_free(data);
    return false;
  }

  if (!stbi_write_png(outfile, width, height, n, data, 0)) {
    std::cerr << "could not write " << outfile << std::endl;
    stbi_image_free(data);
    return false;
  }

  stbi_image_free(data);
  return true;
}


