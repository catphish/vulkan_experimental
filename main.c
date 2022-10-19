#include "engine/engine.h"

int main() {
  struct engine *engine = engine_create();
  engine_run(engine);
  engine_destroy(engine);
  return 0;
}