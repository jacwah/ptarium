#include "camera.h"
#include "file.h"
#include "maths.h"
#include "world.h"
#include "shaders.inc"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_timer.h>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <math.h>
#include <stdio.h>

#define DISPLAY_WIDTH 1080
#define DISPLAY_HEIGHT 720

static bool GlobalUsingMessageCallback;

void GLAPIENTRY
DebugMessageCallback(
        GLenum Source,
        GLenum Type,
        GLuint Id,
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

void
DebugGLError(const char *Filename, int Line)
{
    GLenum Error;

    if (GlobalUsingMessageCallback)
        return;

    while ((Error = glGetError()) != GL_NO_ERROR) {
        const char *ErrorMessage = "Unknown error";

        switch (Error) {
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
        }

        if (ErrorMessage)
            fprintf(stderr,
                    "OpenGL error (0x%X): %s found at %s:%d\n",
                    Error,
                    ErrorMessage,
                    Filename,
                    Line);
    }
}

#define DEBUG_GL() DebugGLError(__FILE__, __LINE__)

GLuint
ShadersCompile()
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

void
MeshSphereCreate(mesh *Mesh, int ParallellCount, int MeridianCount)
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

int
main(int argc, char *argv[])
{
    FILE *File = fopen("planets.csv", "r");
    world *World = (world *) malloc(sizeof(world));
    ReadWorldFile(World, File);
#if 0
    World->Count = 1;
    //World->Name[0] = "Test";
    World->Radius[0] = 50000.0f;
    World->Mass[0] = 1.0f;
    World->Position[0] = glm::vec3(0.0f);
    World->Velocity[0] = glm::vec3(0.0f);
#endif

#if 1
    for (int i = 0; i < World->Count; ++i) {
        World->Radius[i] *= 100.0f;
    }
#endif

    printf("World has %d objects\n", World->Count);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
            SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG | SDL_GL_CONTEXT_DEBUG_FLAG);
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
    if (GLEW_OK != glewInit()) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return 1;
    }

    if (GLEW_KHR_debug) {
        fprintf(stderr, "Debug messages enabled.\n");
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(DebugMessageCallback, 0);
        GlobalUsingMessageCallback = true;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(0.5f);

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

    GLuint VertexBuffers[4];
    glGenBuffers(4, VertexBuffers);

    GLuint AxesVertBuf = VertexBuffers[0];
    GLuint SphereVertBuf = VertexBuffers[1];
    GLuint SphereIndBuf = VertexBuffers[2];
    GLuint LineVertBuf = VertexBuffers[3];

    glBindBuffer(GL_ARRAY_BUFFER, AxesVertBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Axes), Axes, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, SphereVertBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * Sphere.VertexCount, Sphere.Vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereIndBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * Sphere.IndexCount, Sphere.Indices, GL_STATIC_DRAW);

    GLuint ShaderProgram = ShadersCompile();
    glUseProgram(ShaderProgram);

    camera_params CameraParams;
    CameraParams.AspectRatio = (float) DISPLAY_WIDTH / (float) DISPLAY_HEIGHT;
    CameraParams.FovY = glm::radians(80.0f);

    CameraParams.Orientation = {0.0f, PI / 2};
    CameraParams.Distance = 1092.0f;

    GLuint TransformLocation = glGetUniformLocation(ShaderProgram, "Transform");
    GLuint ColorLocation = glGetUniformLocation(ShaderProgram, "Color");
    DEBUG_GL();

    Uint64 PerformanceHz = SDL_GetPerformanceFrequency();
    Uint64 LastTime = SDL_GetPerformanceCounter();
    Uint64 LastPrint = LastTime;
    Uint64 PrintDist = PerformanceHz;
    float FrameLength = 0.0f;

    bool PrintFrameTime = false;
    bool Running = true;
    bool Wireframe = false;
    bool DebugMouseTracing = false;

    int FocusedBody = 0;

    while (Running) {
        SDL_Event Event;
        float dAngle = glm::radians(5.0f);
        bool PrintClickedBody = false;
        while (SDL_PollEvent(&Event)) {
            switch (Event.type) {
                case SDL_QUIT:
                    Running = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    PrintClickedBody = true;
                case SDL_KEYDOWN:
                    {
                    switch (Event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            Running = false;
                            break;
                        case SDLK_UP:
                            CameraParams.Orientation.y += dAngle;
                            break;
                        case SDLK_RIGHT:
                            CameraParams.Orientation.x += dAngle;
                            break;
                        case SDLK_DOWN:
                            CameraParams.Orientation.y -= dAngle;
                            break;
                        case SDLK_LEFT:
                            CameraParams.Orientation.x -= dAngle;
                            break;
                        case SDLK_t:
                            PrintFrameTime = !PrintFrameTime;
                            break;
                        case SDLK_w:
                            Wireframe = !Wireframe;
                            glPolygonMode(GL_FRONT_AND_BACK, Wireframe ? GL_LINE : GL_FILL);
                            break;
                        case SDLK_m:
                            DebugMouseTracing = !DebugMouseTracing;
                            break;
                        case SDLK_0:
                        case SDLK_1:
                        case SDLK_2:
                        case SDLK_3:
                        case SDLK_4:
                        case SDLK_5:
                        case SDLK_6:
                        case SDLK_7:
                        case SDLK_8:
                        case SDLK_9:
                            FocusedBody = Event.key.keysym.sym - SDLK_0;
                            printf("Focus %d\n", FocusedBody);
                            break;
                        case SDLK_PLUS:
                            CameraParams.Distance += 1.0f;
                            printf("Camera distance: %f\n", CameraParams.Distance);
                            break;
                        case SDLK_MINUS:
                            CameraParams.Distance -= 1.0f;
                            printf("Camera distance: %f\n", CameraParams.Distance);
                            break;
                    }
                }
            }
        }

        int MouseX, MouseY;
        SDL_GetMouseState(&MouseX, &MouseY);

        glm::vec2 ScreenPoint(
                (float) MouseX / (float) DISPLAY_WIDTH,
                1.0f - (float) MouseY / (float) DISPLAY_HEIGHT);

#if 0
        for (int i = 0; i < World->Count; ++i) {
            for (int j = 0; j < i; ++j) {
                float G = 6.674e-11;
                glm::vec3 Delta = World->Position[j] - World->Position[i];
                float Distance = glm::length(Delta);
                glm::vec3 Normal = glm::normalize(Delta);
                // TODO: Divide by zero/tiny?
                float ForceWithoutMass = G / (Distance*Distance);
                glm::vec3 DeltaVelocityWithoutMass = ForceWithoutMass*FrameLength*Normal;
                World->Velocity[i] += DeltaVelocityWithoutMass*World->Mass[j];
                World->Velocity[j] -= DeltaVelocityWithoutMass*World->Mass[i];
            }
        }
#endif

#if 0
        for (int i = 0; i < World->Count; ++i) {
            World->Position[i] += World->Velocity[i];
        }
#endif

        if (FocusedBody < World->Count) {
            CameraParams.Focus = World->Position[FocusedBody];
            CameraParams.Distance = 2.0f * World->Radius[FocusedBody];
            CameraParams.NearDistance = 0.9f * World->Radius[FocusedBody];
        }

        camera Camera = CameraParams.MakeCamera();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, SphereVertBuf);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        for (int i = 0; i < World->Count; ++i) {
            glm::mat4 ScaleTransform = glm::scale(glm::mat4(1.0f), glm::vec3(World->Radius[i]));
            glm::mat4 TranslateTransform = glm::translate(glm::mat4(1.0f), World->Position[i]);
            glm::mat4 MVPTransform = Camera.FullTransform * TranslateTransform * ScaleTransform;
            glUniformMatrix4fv(TransformLocation, 1, GL_FALSE, &MVPTransform[0][0]);
            glUniform3f(ColorLocation, World->Color[i].x, World->Color[i].y, World->Color[i].z);
            glDrawElements(GL_TRIANGLES, Sphere.IndexCount, GL_UNSIGNED_SHORT, 0);
        }

        glm::vec3 WorldPointingDir = Camera.WorldDirectionFromScreen(ScreenPoint);

        if (PrintClickedBody) {
            for (int i = 0; i < World->Count; ++i) {
                // TODO: Closest
                int Result = LineSphereIntersect(
                        World->Position[i],
                        World->Radius[i],
                        Camera.Position,
                        WorldPointingDir);
                if (Result == 1)
                    printf("%s\n", World->Name[i]);
            }
        }

        glm::vec3 Line0 = Camera.Position + 2.0f * CameraParams.NearDistance * Camera.LookVector;
        glm::vec3 Line1 = Camera.Position + 2.0f * CameraParams.NearDistance * WorldPointingDir;

        float Line[3*2];
        Line[0] = Line0.x;
        Line[1] = Line0.y;
        Line[2] = Line0.z;
        Line[3] = Line1.x;
        Line[4] = Line1.y;
        Line[5] = Line1.z;

        if (DebugMouseTracing) {
            glDepthFunc(GL_ALWAYS);
            glUniformMatrix4fv(TransformLocation, 1, GL_FALSE, &Camera.FullTransform[0][0]);
            glUniform3f(ColorLocation, 1.0f, 0.0f, 1.0f);
            glBindBuffer(GL_ARRAY_BUFFER, LineVertBuf);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Line), Line, GL_STREAM_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glDrawArrays(GL_LINES, 0, 2);
            glDepthFunc(GL_LESS);
        }

        /*
        glUniformMatrix4fv(TransformLocation, 1, GL_FALSE, &AxesView[0][0]);
        glBindBuffer(GL_ARRAY_BUFFER, AxesVertBuf);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_LINES, 0, 6);
        */

        glDisableVertexAttribArray(0);

        DEBUG_GL();

        Uint64 CurrentTime = SDL_GetPerformanceCounter();
        FrameLength = (CurrentTime - LastTime) / (float)PerformanceHz;
        if (PrintFrameTime && CurrentTime > LastPrint + PrintDist) {
            float FrameMs = 1000.0f * FrameLength;
            printf("%.2f\n", FrameMs);
            LastPrint = CurrentTime;
        }
        LastTime = CurrentTime;

        SDL_GL_SwapWindow(Window);
    }

    return 0;
}
