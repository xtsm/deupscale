#include "deupscale_file.h"
#include <iostream>

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " INPUT OUTPUT" << std::endl;
    return 1;
  }
  DeupscaleFile(argv[1], argv[2]);
}
