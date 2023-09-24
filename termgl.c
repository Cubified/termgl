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

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#define GL_SILENCE_DEPRECATION
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
    msg_buf = malloc(max_length); \
    glGetShaderInfoLog(shader, max_length, &max_length, msg_buf); \
    fprintf(stderr, "%s", msg_buf); \
    glDeleteShader(shader); \
    free(msg_buf); \
    return 4; \
  }

#define COORDINATE_DECODE() \
  tok = strtok(NULL, ";"); \
  args->x = ((float)atoi(tok)) / (2.0f * (float)dims.w); \
  tok = strtok(NULL, ";"); \
  args->y = 2.0f * ((float)strtol(tok, NULL, 10)) / ((float)dims.h)

#define FPS 60.0f

typedef struct args_t {
  float x, y, down;
} args_t;
 
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
char *seq;
 
static const char *vertex_shader_text =
"#version 110\n"
"attribute vec2 aPos;\n"
"varying vec2 vPos;\n"
"void main() {\n"
"  gl_Position = vec4(aPos, 0.0, 1.0);\n"
"  vPos = aPos + 1.0;\n"
"}\n";
static const char *fragment_shader_prefix =
"const vec2 iResolution = vec2(1.0, 1.0);\n"
"uniform vec3 iMouse;\n"
"uniform float iTime;\n"
"varying vec2 vPos;\n";
static const char *fragment_shader_suffix =
"\n"
"void main() {\n"
"  vec4 fragColor = vec4(0.0);\n"
"  mainImage(fragColor, vPos);\n"
"  gl_FragColor = fragColor;\n"
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
  dims.w = ws.ws_col / 2;
  dims.h = ws.ws_row;
  if (sig == -1) return;

  glfwSetWindowSize(window, dims.w, dims.h);
  glfwGetFramebufferSize(window, &fb_w, &fb_h);
  glViewport(0, 0, fb_w, fb_h);
 
  if (pixels == NULL) {
    pixels = malloc(dims.w * dims.h * 4);
    seq = malloc(dims.w * dims.h * 22);
  } else {
    pixels = realloc(pixels, dims.w * dims.h * 4);
    seq = realloc(seq, dims.w * dims.h * 22);
  }
}

void loop(void *raw_args) {
  args_t *args = (args_t*)raw_args;
  char buf[64];
  char cpy[64];
  char *tok;
  int n;

  while ((n=read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
    if (n >= 8 && buf[0] == '\x1b' && buf[1] == '[' && buf[2] == '<') {
      strncpy(cpy, buf, n);
      tok = strtok(cpy+3, ";");

      switch (tok[0]) {
        case '0':
          args->down = (strchr(cpy+5, 'm') == NULL) ? 1.0f : 0.0f;
          break;
        case '3':
          args->down = (tok[1] == '2') ? 1.0f : 0.0f;
          break;
        default:
          continue;
      }

      COORDINATE_DECODE();
    }
  }
}
 
int main(int argc, char **argv) {
  GLuint vertex_buffer, vertex_shader, fragment_shader, program;
  GLint apos_location, itime_location, imouse_location, is_compiled, max_length;
  GLenum err;
  struct termios raw, tio;
  char *fragment_shader_base, *fragment_shader_text, *term, buf[64];
  GLchar *msg_buf;
  int filesize, n, i;
  FILE *fp;
  double time = 0.0f, last = 0.0;

  if (argc != 2) {
    printf("Usage: termgl [fragment shader]\n");
    return 0;
  }

  /** Raw mode/signals **/
  tcgetattr(STDIN_FILENO, &tio);
  raw = tio;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  signal(SIGINT, stop);
  signal(SIGTERM, stop);
  signal(SIGWINCH, resize);

  resize(-1);

  /** OpenGL context and buffers **/
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

  /** Shader compilation **/
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
  fragment_shader_base = malloc((filesize=ftell(fp)));
  fseek(fp, 0, SEEK_SET);
  fread(fragment_shader_base, filesize, 1, fp);
  fragment_shader_base[filesize] = '\0';

  fragment_shader_text = malloc(
    strlen(fragment_shader_prefix) + filesize + strlen(fragment_shader_suffix)
  );
  sprintf(
    fragment_shader_text,
    "%s%s%s",
    fragment_shader_prefix,
    fragment_shader_base,
    fragment_shader_suffix
  );

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, (const GLchar *const *)&fragment_shader_text, NULL);
  glCompileShader(fragment_shader);
  glCheckErrors();

  /** Program, attributes, and uniforms **/
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  glCheckErrors();

  glVerifyCompilation(vertex_shader);
  glVerifyCompilation(fragment_shader);

  glUseProgram(program);

  apos_location = glGetAttribLocation(program, "aPos");
  itime_location = glGetUniformLocation(program, "iTime");
  imouse_location = glGetUniformLocation(program, "iMouse");
  glCheckErrors();

  glEnableVertexAttribArray(apos_location);
  glVertexAttribPointer(apos_location, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*)0);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glCheckErrors();
  resize(1);

  /** User input thread **/
#ifdef HAVE_PTHREAD_H
  args_t thread_args = { 0.0f, 0.0f, 0.0f };
  pthread_t main_thread;
  pthread_create(
    &main_thread,
    NULL,
    loop,
    &thread_args
  );
#endif

  /** Render loop **/
  printf("\x1b[?1049h\x1b[0m\x1b[2J\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");
  while (running) {
    time = glfwGetTime();
    if (time - last < 1.0f / FPS) continue;
    last = time;

    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1f(itime_location, time);
#ifdef HAVE_PTHREAD_H
    glUniform3f(imouse_location, thread_args.x, thread_args.y, thread_args.down);
#endif
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glReadPixels(0, 0, dims.w, dims.h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    i = sprintf(seq, "\x1b[2J\x1b[0H\x1b[0m");
    for (int y = 0; y < dims.h - 1; y++) {
      for (int x = 0; x < dims.w; x++) {
        int index = ((dims.h - y) * dims.w + x) * 4;
        GLubyte red = pixels[index] & 0xff;
        GLubyte green = pixels[index + 1] & 0xff;
        GLubyte blue = pixels[index + 2] & 0xff;

        i += sprintf(seq + i, "\x1b[48;2;%u;%u;%um  ", red, green, blue);
      }
      i += sprintf(seq + i, "\n");
    }
    puts(seq);
    fflush(stdout);

    glfwSwapBuffers(window);
  }
  printf("\x1b[0m\x1b[2J\x1b[?1049l\x1b[?1003l\x1b[?1015l\x1b[?1006l\x1b[?25h");

  /** Cleanup **/
  free(seq);
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
