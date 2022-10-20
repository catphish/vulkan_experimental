#include "engine/engine.h"

int main() {
  Engine *engine = engineCreate();
  engineAddPipeline(engine, pipelineCreate(engine));
  engineRun(engine);
  engineDestroy(engine);
  return 0;
}