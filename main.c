#include "engine/engine.h"

int main() {
  Engine *engine = engineCreate();
  engineRun(engine);
  engineDestroy(engine);
  return 0;
}