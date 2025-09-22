#include "vklite.h"
#include <iostream>

namespace vklite {

bool Context::initialize(const std::string &appName){
  std::cout << "vklite: initialize for " << appName;
#if defined(VKLITE_PLAT_WINDOWS)
  std::cout << " [platform: windows]";
#elif defined(VKLITE_PLAT_MAC)
  std::cout << " [platform: macos]";
#elif defined(VKLITE_PLAT_LINUX)
  std::cout << " [platform: linux]";
#endif
  std::cout << "\n";
  // Real initialization should create a VkInstance, enable desired layers
  // and extensions, and prepare platform surfaces. Implementation is
  // intentionally left as the library's real work.
  return true;
}

void Context::shutdown(){
  std::cout << "vklite: shutdown\n";
}

} // namespace vklite
