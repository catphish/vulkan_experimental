CFLAGS = -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXrandr -lcglm -lm -lstb -lassimp

test: Vulkan
	./Vulkan

shaders/triangle.vert.spv: shaders/triangle.vert
	glslc shaders/triangle.vert -o shaders/triangle.vert.spv

shaders/triangle.frag.spv: shaders/triangle.frag
	glslc shaders/triangle.frag -o shaders/triangle.frag.spv

shaders: shaders/triangle.vert.spv shaders/triangle.frag.spv

Vulkan: shaders main.c engine/*.c
	gcc $(CFLAGS) -o Vulkan main.c engine/*.c $(LDFLAGS)

.PHONY: clean

clean:
	rm -f Vulkan
