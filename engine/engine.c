#include "engine.h"

#include <stdio.h>
#include <stdlib.h>

#include "window.h"

struct engine *engine_create(void) {
  struct engine *engine;
  engine = malloc(sizeof(*engine));
  engine->window = window_create();

  return engine;
}

void engine_run(struct engine *engine) {
  while (!glfwWindowShouldClose(engine->window)) {
    glfwPollEvents();
  }
}

void engine_destroy(struct engine *engine) { free(engine); }
