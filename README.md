# termgl

A terminal-based renderer for OpenGL shaders.  Like [Shadertoy](https://shadertoy.com), but in the terminal.

This project uses [GLFW](https://www.glfw.org) to render an invisible OpenGL context, then converts its pixel data to ANSI escape sequences understood by a terminal emulator.

Currently, it supports rendering custom fragment shaders to a single quad that fills the entire terminal window.  Support for custom vertex shaders (or geometries) is planned.

## Demos

- [Psychedelix](https://www.shadertoy.com/view/MdsXDM):

![demo-1.gif](https://github.com/Cubified/termgl/raw/main/img/demo-1.gif)

- [Diamonds and Diagonals](https://www.shadertoy.com/view/DtyXDR):

![demo-2.gif](https://github.com/Cubified/termgl/raw/main/img/demo-2.gif)

- [Noisy Gradient](https://andrewlrussell.com):

![demo-3.gif](https://github.com/Cubified/termgl/raw/main/img/demo-3.gif)

- [Metaballs](https://www.shadertoy.com/view/ld2GRz):

![demo-4.gif](https://github.com/Cubified/termgl/raw/main/img/demo-4.gif)

- [Happy Jumping](https://www.shadertoy.com/view/3lsSzf)

![demo-5.gif](https://github.com/Cubified/termgl/raw/main/img/demo-5.gif)

## Compiling and Running

Prior to compiling, ensure that [GLFW](https://www.glfw.org) has been installed.  Then, run:

```sh
$ make
$ ./termgl demos/1-basic.frag
```

Assuming a successful compilation, `termgl` should display the following:

![basic.png](https://github.com/Cubified/termgl/raw/main/img/basic.png)

To close the application, send a `SIGTERM` with `Ctrl+C`.

## Running A Custom Shader

To run a custom fragment shader, simply pass its filename as the first argument to `termgl`.  A minimal shader is as follows:

```glsl
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
  fragColor = vec4(fragCoord, 0.5, 1.0);
}
```

## Running Shaders Directly from Shadertoy

`termgl` exposes a minimal wrapper that emulates Shadertoy's uniforms and attributes.  For *most* (but not all) shaders, pasting the contents into a file and opening them with `termgl` should work properly.  In particular, `termgl` exposes:

```glsl
uniform vec2 iResolution;
uniform vec3 iMouse;
uniform float iTime;
```

None of these need to be included in the shader code, `termgl`'s wrapper adds these lines automatically (just as Shadertoy does).

Others (such as `iDate`) are not yet supported.

## Notes

- `termgl` should work on any platform supported by GLFW including Linux, MacOS, and Windows.  However, compilation on Linux and Windows (or any other platform) is untested.
- Running `termgl` within a terminal multiplexer such as [`tmux`](https://github.com/tmux/tmux) is not recommended for performance reasons.  For best performance, run `termgl` natively inside a GPU-accelerated terminal such as [kitty](https://sw.kovidgoyal.net/kitty) or [alacritty](https://github.com/alacritty/alacritty).
- The uniform `iMouse` is not yet fully supported, so its value defaults to `vec3(0.0, 0.0, 0.0)`.
- The GLSL version is system-dependent, but all shaders in `demo/` are confirmed to be working with GLSL 1.10 (i.e. `#version 110`).

## To-Do

- Add the ability to select a different geometry on which to render the shader (e.g. cube, sphere, etc.)
- Investigate occasional crashes on window resize
- Add more uniforms and other quality of life improvements
- Add support for channels (i.e. custom images/textures) and interactivity (i.e. keyboard and mouse)
- Add mouse support via `iMouse`

## A Few of My Other Terminal Projects

- [tuibox](https://github.com/Cubified/tuibox):  A single-header terminal UI library
- [vex](https://github.com/Cubified/vex):  A terminal-based hex editor with vi-like keybinds
- [colorslide](https://github.com/Cubified/colorslide):  A terminal-based color picker with mouse support
