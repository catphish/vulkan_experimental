CFLAGS = -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXrandr -lcglm -lm -lstb -lassimp

test: Vulkan
	./Vulkan

Vulkan: main.c engine/engine.c engine/window.c
	gcc $(CFLAGS) -o Vulkan main.c engine/*.c $(LDFLAGS)

.PHONY: clean

clean:
	rm -f Vulkan
