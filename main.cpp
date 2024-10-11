// Illtyd Wynn, 8/26/2024, Vulkan Learning

/*
  In a similar vein to my DX11 learnings, I will
  write this in my own sort of pseudo-C style. I will
  also be adamant about breaking this down into
  different files to make management easier. I want
  to sort of "buck" the tutorials so I have to think
  about how the pieces fit together more.

  // TODO: Current chapter: https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain
*/

#include "application.h"
#include <iostream>

using namespace std;

int main() {
  application app;

  try {
    run_application(&app);
  } catch (const exception& e) {
    cerr << e.what() << endl;
    return -1;
  }

  return 0;
}
