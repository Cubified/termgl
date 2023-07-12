/*
 * termgl.c: A program for rendering OpenGL shaders in the terminal.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <math.h>

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define glCheckErrors() \
  if ((err = glGetError()) != GL_NO_ERROR) { \
    fprintf(stderr, "Error: OpenGL code 0x%x (%s:%d)\n", err, __FILE__, __LINE__); \
    return 4; \
  }

#define glVerifyCompilation(shader) \
  glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled); \
  if (is_compiled == GL_FALSE) { \
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length); \
    glGetShaderInfoLog(shader, max_length, &max_length, msg_buf); \
    fprintf(stderr, "%s", msg_buf); \
    glDeleteShader(shader); \
    return 4; \
  }

#define FPS 60.0f
 
static const struct {
  float x, y;
} vertices[6] = {
  { -1.0f, -1.0f },
  { -1.0f,  1.0f },
  {  1.0f,  1.0f },
  { -1.0f, -1.0f },
  {  1.0f,  1.0f },
  {  1.0f, -1.0f },
};

static struct {
  int w, h;
} dims = {
  0, 0
};

GLFWwindow *window;
GLubyte *pixels = NULL;
struct winsize ws;
int running = 1;
 
static const char *vertex_shader_text =
"#version 110\n"
"attribute vec2 a_pos;\n"
"varying vec2 v_pos;\n"
"void main() {\n"
"  gl_Position = vec4(a_pos, 0.0, 1.0);\n"
"  v_pos = a_pos + 1.0;\n"
"}\n";
 
static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}
 
void stop(int sig) {
  running = 0;
}
void resize(int sig) {
  int fb_w, fb_h;

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  dims.w = ws.ws_col;
  dims.h = ws.ws_row;
  if (sig == -1) return;

  glfwSetWindowSize(window, dims.w, dims.h);
  glfwGetFramebufferSize(window, &fb_w, &fb_h);
  glViewport(0, 0, fb_w, fb_h);
 
  if (pixels == NULL) {
    pixels = malloc(dims.w * dims.h * 4);
  } else {
    pixels = realloc(pixels, dims.w * dims.h * 4);
  }
}
 
int main(int argc, char **argv) {
  GLuint vertex_buffer, vertex_shader, fragment_shader, program;
  GLint apos_location, utime_location, is_compiled, max_length;
  GLenum err;
  struct termios raw, tio;
  char *fragment_shader_text, *term;
  GLchar msg_buf[512];
  int filesize;
  double time = 0.0f, last = 0.0;
  FILE *fp;

  if (argc != 2) {
    printf("Usage: termgl [fragment shader]\n");
    return 0;
  }

  tcgetattr(STDIN_FILENO, &tio);
  raw = tio;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  signal(SIGINT, stop);
  signal(SIGTERM, stop);
  signal(SIGWINCH, resize);

  resize(-1);

  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    fprintf(stderr, "Error: Failed to initialize GLFW.\n");
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

  window = glfwCreateWindow(dims.w, dims.h, "termgl", NULL, NULL);
  if (!window) {
    glfwTerminate();
    fprintf(stderr, "Error: Failed to create GLFW window.\n");
    return 2;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  resize(0);

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glCheckErrors();

  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);
  glCheckErrors();

  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Error: Unable to open \"%s\" for reading.\n", argv[1]);
    return 3;
  }
  fseek(fp, 0, SEEK_END);
  fragment_shader_text = malloc((filesize=ftell(fp)));
  fseek(fp, 0, SEEK_SET);
  fread(fragment_shader_text, filesize, 1, fp);
  fragment_shader_text[filesize] = '\0';

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, (const GLchar *const *)&fragment_shader_text, NULL);
  glCompileShader(fragment_shader);
  glCheckErrors();

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  glCheckErrors();

  glVerifyCompilation(vertex_shader);
  glVerifyCompilation(fragment_shader);

  glUseProgram(program);

  apos_location = glGetAttribLocation(program, "a_pos");
  utime_location = glGetUniformLocation(program, "u_time");
  glCheckErrors();

  glEnableVertexAttribArray(apos_location);
  glVertexAttribPointer(apos_location, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*)0);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glCheckErrors();

  printf("\x1b[?1049h\x1b[0m\x1b[2J\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");
  while (running) {
    time = glfwGetTime();
    if (time - last < 1.0f / FPS) continue;
    last = time;

    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1f(utime_location, time);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glReadPixels(0, 0, dims.w, dims.h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    printf("\x1b[0;0H");
    for (int y = 0; y < dims.h; y++) {
      for (int x = 0; x < dims.w; x++) {
        int index = (y * dims.w + x) * 4;
        GLubyte red = pixels[index];
        GLubyte green = pixels[index + 1];
        GLubyte blue = pixels[index + 2];

        printf("\x1b[48;2;%d;%d;%dm ", red, green, blue);
      }
      printf("\x1b[%d;1H", y);
    }
    fflush(stdout);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  printf("\x1b[0m\x1b[2J\x1b[?1049l\x1b[?1003l\x1b[?1015l\x1b[?1006l\x1b[?25h");

  free(pixels);
  free(fragment_shader_text);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tio);

  glfwDestroyWindow(window);
  glfwTerminate();

  term = getenv("TERM");
  if(strncmp(term, "screen", 6) == 0 ||
     strncmp(term, "tmux", 4) == 0 ||
     getenv("TMUX") != NULL
  ){
    printf("Note: Terminal multiplexer detected.\n  For best performance (i.e. reduced flickering), running natively inside\n  a GPU-accelerated terminal such as alacritty or kitty is recommended.\n");
  }

  return 0;
}
