// Based on the texture demo by Dr. Palmer
// ezview - Christopher Philabaum <cp723@nau.edu>
// Allows the user to load in an image and perform various affine transformations
// on it

#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <linmath.h>
#include "read.h"

typedef struct {
    float Position[2];
    float TexCoord[2];
} Vertex;

Vertex vertexes[] = {
    {{1, -1}, {0.99999, 0.99999}}, // Bottom-right
    {{1, 1},  {0.99999, 0}}, // Top-right
    {{-1, -1}, {0, 0.99999}}, // Bottom-left
    {{-1, 1}, {0, 0}}// Top-left
};

#define ANGLE_STEP 1.5707963267948966192313216916398
#define TRANSLATE_STEP 0.2
#define SCALE_STEP 2
#define SHEAR_STEP 0.1

mat4x4 matrix;
float angle;

static void matrix_reset() {
    mat4x4_identity(matrix);
}

static const char* vertex_shader_src =
    "uniform mat4 MVP;\n"
    "attribute vec2 TexCoordIn;\n"
    "attribute vec2 vPos;\n"
    "varying vec2 TexCoordOut;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
    "    TexCoordOut = TexCoordIn;\n"
    "}\n";

static const char* fragment_shader_src =
    "varying highp vec2 TexCoordOut;\n"
    "uniform sampler2D Texture;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
    "}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    mat4x4 transform_m;
    mat4x4_identity(transform_m);

    // Exit
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    // Rotate left
    if(key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
        mat4x4_rotate_Z(matrix, matrix, ANGLE_STEP);
    }
    // Rotate right
    if(key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
        mat4x4_rotate_Z(matrix, matrix, -ANGLE_STEP);
    }
    // Shear parallel to x
    if(key == GLFW_KEY_SEMICOLON && action == GLFW_PRESS) {
        transform_m[1][0] = -SHEAR_STEP;
    }
    if(key == GLFW_KEY_APOSTROPHE && action == GLFW_PRESS) {
        transform_m[1][0] = SHEAR_STEP;
    }
    // Shear parallel to y
    if(key == GLFW_KEY_PERIOD && action == GLFW_PRESS) {
        transform_m[0][1] = -SHEAR_STEP;
    }
    if(key == GLFW_KEY_SLASH && action == GLFW_PRESS) {
        transform_m[0][1] = SHEAR_STEP;
    }
    // Zoom in
    if(key == GLFW_KEY_EQUAL && action == GLFW_PRESS) {
        transform_m[0][0] = SCALE_STEP;
        transform_m[1][1] = SCALE_STEP;
    }
    // Zoom out
    if(key == GLFW_KEY_MINUS && action == GLFW_PRESS) {
        transform_m[0][0] = 1 / (float)SCALE_STEP;
        transform_m[1][1] = 1 / (float)SCALE_STEP;
    }
    // Translate image
    if(key == GLFW_KEY_UP && action == GLFW_PRESS) {
        mat4x4_translate(transform_m, 0, TRANSLATE_STEP, 0);
    }
    if(key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        mat4x4_translate(transform_m, -TRANSLATE_STEP, 0, 0);
    }
    if(key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        mat4x4_translate(transform_m, 0, -TRANSLATE_STEP, 0);
    }
    if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        mat4x4_translate(transform_m, TRANSLATE_STEP, 0, 0);
    }
    // Reset
    if(key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        matrix_reset(matrix);
    }

    mat4x4_mul(matrix, matrix, transform_m);
}

void glCompileShaderOrDie(GLuint shader) {
    GLint compiled;

    glCompileShader(shader);
    glGetShaderiv(shader,
        GL_COMPILE_STATUS,
        &compiled);

    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader,
            GL_INFO_LOG_LENGTH,
            &infoLen);
        char* info = malloc(infoLen+1);
        GLint done;
        glGetShaderInfoLog(shader, infoLen, &done, info);
        fprintf(stderr, "Unable to compile shader: %s\n", info);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char* argv[])
{
    // Load PPM file
    if(argc != 2) {
        fprintf(stderr, "usage: ezview /path/to/inputFile\n");
        return EXIT_FAILURE;
    }

    const char* inputPath = argv[1];

    FILE* inputFd;

    if((inputFd = fopen(inputPath, "r")) == NULL) {
        perror("Error: Cannot open input file\n");
        return EXIT_FAILURE;
    }

    pnmHeader header;
    pixel* pixels;

    // Read the file, get format
    if(readHeader(&header, inputFd) < 0) {
        return EXIT_FAILURE;
    }

    if(header.mode != 3 || header.mode != 6) {
        fprintf(stderr, "Error: PPM file must be P3 or P6.\n");
        return EXIT_FAILURE;
    }

    if((pixels = malloc(sizeof(*pixels) * header.width * header.height)) == NULL) {
        perror("Error: Memory allocation error on pixels\n");
        return EXIT_FAILURE;
    }
    if(readBody(header, pixels, inputFd) < 0) {
        return EXIT_FAILURE;
    }

    if(fclose(inputFd) == EOF) {
        perror("Error: Closing file\n");
        return EXIT_FAILURE;
    }

    // OpenGL Start
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1024, 768, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stderr, "Error: glfwCreateWindow\n");
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    // gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
    glCompileShaderOrDie(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
    glCompileShaderOrDie(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    // more error checking! glLinkProgramOrDie!

    mvp_location = glGetUniformLocation(program, "MVP");
    assert(mvp_location != -1);

    vpos_location = glGetAttribLocation(program, "vPos");
    assert(vpos_location != -1);

    GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
    assert(texcoord_location != -1);

    GLint tex_location = glGetUniformLocation(program, "Texture");
    assert(tex_location != -1);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*) 0);

    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*) (sizeof(float) * 2));

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, header.width, header.height, 0, GL_RGB,
        GL_UNSIGNED_BYTE, pixels);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex_location, 0);

    // Initialize matrix
    matrix_reset(matrix);

    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;
        mat4x4 p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, matrix);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
