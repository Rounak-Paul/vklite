#include "vklite.h"
#include <iostream>

int main(){
  vklite::Context ctx;
  if(!ctx.initialize("sandbox")){
    std::cerr << "Failed to initialize vklite\n";
    return 1;
  }

  std::cout << "sandbox running...\n";

  ctx.shutdown();
  return 0;
}
