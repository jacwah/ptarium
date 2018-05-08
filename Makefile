CFLAGS ?= -g -std=c++11
LFLAGS = -lGLEW -framework OpenGL -lSDL2

TARGET = ptarium
SHADER_TARGET = shaders.inc

SOURCE = ptarium.cpp
SHADER = shader.vert shader.frag

all: $(TARGET)

$(TARGET): $(SOURCE) $(SHADER_TARGET)
	@$(CXX) $(CFLAGS) -o $@ $(SOURCE) $(LFLAGS)

$(SHADER_TARGET): $(SHADER)
	@-rm $@
	@for file in $(SHADER); do xxd -i $$file >> $@; done

clean:
	@$(RM) $(TARGET)
	@$(RM) -r $(TARGET).dSYM
	@$(RM) $(SHADER_TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
