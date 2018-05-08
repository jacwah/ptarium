#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include "shaders.inc"

#define DISPLAY_WIDTH 960
#define DISPLAY_HEIGHT 540

void DebugMessageCallback(
        GLenum Source,
        GLenum Type,
        GLenum Id,
        GLenum Severity,
        GLsizei Length,
        const GLchar *Message,
        const void *UserParam)
{
    fprintf(stderr,
            "OpenGL message (type 0x%X, severity 0x%X): %s\n",
            Type,
            Severity,
            Message);
}

void DebugError(const char *Filename, int Line)
{
    GLenum Error = glGetError();
    const char *ErrorMessage = NULL;

    switch (Error) {
        case GL_NO_ERROR:
            break;
        case GL_INVALID_ENUM:
            ErrorMessage = "Invalid enum";
            break;
        case GL_INVALID_VALUE:
            ErrorMessage = "Invalid value";
            break;
        case GL_INVALID_OPERATION:
            ErrorMessage = "Invalid operation";
            break;
        case GL_STACK_OVERFLOW:
            ErrorMessage = "Stack overflow";
            break;
        case GL_STACK_UNDERFLOW:
            ErrorMessage = "Stack underflow";
            break;
        case GL_OUT_OF_MEMORY:
            ErrorMessage = "Out of memory";
            break;
        case GL_TABLE_TOO_LARGE:
            ErrorMessage = "Table too large";
            break;
        default:
            ErrorMessage = "Unknown error";
            break;
    }

    if (ErrorMessage)
        fprintf(stderr,
                "OpenGL error (0x%X): %s found at %s:%d\n",
                Error,
                ErrorMessage,
                Filename,
                Line);
}

#define DEBUGERR() DebugError(__FILE__, __LINE__)

GLuint ShadersCompile()
{
    GLchar *VertexSource = (GLchar *) shader_vert;
    GLchar *FragmentSource = (GLchar *) shader_frag;

    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexSource, (GLint *) &shader_vert_len);
    glCompileShader(VertexShader);

    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentSource, (GLint *) &shader_frag_len);
    glCompileShader(FragmentShader);

    GLint Status;
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Status);
    if (GL_TRUE != Status)
        printf("Vertex shader failed to compile\n");
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Status);
    if (GL_TRUE != Status)
        printf("Fragment shader failed to compile\n");

    GLuint Program = glCreateProgram();
    glAttachShader(Program, VertexShader);
    glAttachShader(Program, FragmentShader);
    glLinkProgram(Program);

    glGetProgramiv(Program, GL_LINK_STATUS, &Status);
    if (GL_TRUE != Status)
        printf("Shader program failed to link\n");

    return Program;
}

struct mesh {
    GLuint VertexCount;
    GLfloat *Vertices;
    GLuint IndexCount;
    GLushort *Indices;
};

#define PI 3.1415f

void MeshSphereCreate(mesh *Mesh, int ParallellCount, int MeridianCount)
{
    Mesh->VertexCount = 3 * (ParallellCount * MeridianCount + 2);
    Mesh->Vertices = (GLfloat *) malloc(sizeof(GLfloat) * Mesh->VertexCount);
    Mesh->IndexCount = 3 * (2 * MeridianCount + 2 * ParallellCount * MeridianCount);
    Mesh->Indices = (GLushort *) malloc(sizeof(GLushort) * Mesh->IndexCount);
    GLfloat *Vertex = Mesh->Vertices;
    GLushort *Index = Mesh->Indices;

    // MeridianCount triangles at top
    // ParallellCount * MeridianCount * 2 triangle in body
    // MeridianCount triangles at bottom

    for (int TrigIndex = 0;
            TrigIndex < MeridianCount - 1;
            TrigIndex++) {
        *Index++ = 0;
        *Index++ = TrigIndex + 2;
        *Index++ = TrigIndex + 1;
    }

    // Last one should wrap
    *Index++ = 0;
    *Index++ = 1;
    *Index++ = MeridianCount;

    for (int ParallellIndex = 1;
            ParallellIndex < MeridianCount - 1;
            ParallellIndex++) {
        GLushort Base1 = (ParallellIndex-1) * MeridianCount + 1;
        GLushort Base2 = ParallellIndex * MeridianCount + 1;

        for (int MeridianIndex = 0;
                MeridianIndex < MeridianCount;
                MeridianIndex++) {
            GLushort NextMeridian = (MeridianIndex + 1) % MeridianCount;

            *Index++ = Base1 + MeridianIndex;
            *Index++ = Base2 + NextMeridian;
            *Index++ = Base2 + MeridianIndex;

            *Index++ = Base1 + MeridianIndex;
            *Index++ = Base1 + NextMeridian;
            *Index++ = Base2 + NextMeridian;
        }
    }

    GLshort SouthPole = MeridianCount * ParallellCount - 1;
    GLshort BottomBase = MeridianCount * (ParallellCount - 2);
    for (int TrigIndex = 0;
            TrigIndex < MeridianCount;
            TrigIndex++) {
        *Index++ = SouthPole;
        *Index++ = BottomBase + TrigIndex + 1;
        *Index++ = BottomBase + (TrigIndex+1)%MeridianCount + 1;
    }

    *Vertex++ = 0.0f;
    *Vertex++ = 1.0f;
    *Vertex++ = 0.0f;

    for (int ParallellIndex = 0;
            ParallellIndex < ParallellCount;
            ParallellIndex++) {
        GLfloat Parallell = PI * (ParallellIndex + 1) / (GLfloat) ParallellCount;
        for (int MeridianIndex = 0;
                MeridianIndex < MeridianCount;
                MeridianIndex++) {
            GLfloat Meridian = 2.0f * PI * MeridianIndex / MeridianCount;
            *Vertex++ = sinf(Parallell) * cosf(Meridian);
            *Vertex++ = cosf(Parallell);
            *Vertex++ = sinf(Parallell) * sinf(Meridian);
        }
    }

    *Vertex++ = 0.0f;
    *Vertex++ = -1.0f;
    *Vertex++ = 0.0f;
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_FLAGS,
            SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_Window *Window = SDL_CreateWindow(
            "ptarium",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            DISPLAY_WIDTH,
            DISPLAY_HEIGHT,
            SDL_WINDOW_OPENGL);
    SDL_GLContext GLContext = SDL_GL_CreateContext(Window);

    // Also load extensions not reported by driver
    glewExperimental = GL_TRUE;
    glewInit();

    // Wtf apple
    //glEnable(GL_DEBUG_OUTPUT);
    //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    //glDebugMessageCallback(DebugMessageCallback, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    float Points[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    float TriangleVertices[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
    };

    float Axes[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    mesh Sphere;
    MeshSphereCreate(&Sphere, 20, 20);

    GLuint VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    GLuint VertexBuffers[3];
    glGenBuffers(3, VertexBuffers);

    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Axes), Axes, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * Sphere.VertexCount, Sphere.Vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VertexBuffers[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * Sphere.IndexCount, Sphere.Indices, GL_STATIC_DRAW);

    GLuint ShaderProgram = ShadersCompile();
    glUseProgram(ShaderProgram);

    glm::mat4 Perspective = glm::perspective(
            glm::radians(45.0f),
            (float) DISPLAY_WIDTH / (float) DISPLAY_HEIGHT,
            0.1f,
            100.0f);
    glm::mat4 View = glm::lookAt(
            glm::vec3(2.0f, 1.0f, 2.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

    GLuint TransformLocation = glGetUniformLocation(ShaderProgram, "Transform");

    DEBUGERR();

    bool Running = true;
    while (Running) {
        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {
            switch (Event.type) {
                case SDL_QUIT:
                    Running = false;
                    break;
                case SDL_KEYDOWN:
                    {
                    glm::vec4 Axis4 = glm::inverse(View) * glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
                    switch (Event.key.keysym.sym) {
                        case SDLK_UP:
                            {
                            //glm::vec3 Axis3(Axis4.x, 0.0f, Axis4.z);
                            glm::vec3 Axis3(Axis4.x, 0.0f, 0.0f);
                            View = glm::rotate(View, glm::radians(5.0f), Axis3);
                            }
                            break;
                        case SDLK_RIGHT:
                            {
                            glm::vec3 Axis3(0.0f, Axis4.y, 0.0f);
                            View = glm::rotate(View, glm::radians(5.0f), Axis3);
                            }
                            //View = glm::rotate(View, glm::radians(1.0f), View * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
                            break;
                        case SDLK_DOWN:
                            {
                            //glm::vec3 Axis3(-Axis4.x, 0.0f, -Axis4.z);
                            glm::vec3 Axis3(-Axis4.x, 0.0f, 0.0f);
                            View = glm::rotate(View, glm::radians(5.0f), Axis3);
                            }
                            //View = glm::rotate(View, glm::radians(1.0f), View * glm::vec4(-1.0f, 0.0f, -1.0f, 0.0f));
                            break;
                        case SDLK_LEFT:
                            {
                            glm::vec3 Axis3(0.0f, -Axis4.y, 0.0f);
                            View = glm::rotate(View, glm::radians(5.0f), Axis3);
                            }
                            //View = glm::rotate(View, glm::radians(1.0f), View * glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
                            break;
                    }
                }
            }
        }

        glm::mat4 Transform = Perspective * View;
        glUniformMatrix4fv(TransformLocation, 1, GL_FALSE, &Transform[0][0]);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffers[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_LINES, 0, 6);

        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffers[1]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawElements(GL_TRIANGLES, Sphere.IndexCount, GL_UNSIGNED_SHORT, 0);
        //glDrawArrays(GL_LINE_STRIP, 0, 1 + 10 * 10);
        //glDrawArrays(GL_POINTS, 0, 10 * 10);
        //glDrawArrays(GL_TRIANGLES, 0, 3);

        glDisableVertexAttribArray(0);

        DEBUGERR();
        SDL_GL_SwapWindow(Window);
    }
}
