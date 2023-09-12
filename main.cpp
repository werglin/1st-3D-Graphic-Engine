#include "VulkanRenderer.h"

#include <iostream>
#include <cstdlib>

// this is for git check
int main() {
    
    VulkanRenderer renderer;

    try {
        renderer.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}