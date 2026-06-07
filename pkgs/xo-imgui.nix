{
  # nixpkgs dependencies
  lib, stdenv, cmake, pkg-config,
  vulkan-headers,
  vulkan-loader,
  SDL2,
  xorg,

  # opengl deps
  glew,
  libGL,
#  mesa-demos,

  # xo dependencies
  xo-randomgen ? null,
  xo-object ? null,
  xo-indentlog,
  xo-cmake,

  buildExamples ? false,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-imgui";

    src = ../xo-imgui;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"
                  "-DXO_ENABLE_VULKAN=on"
                  "-DXO_ENABLE_OPENGL=on"
                 ]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"];

    nativeBuildInputs = [
      cmake pkg-config xo-cmake
    ] ++ lib.optionals buildExamples [
      xo-randomgen
      xo-object
    ];

    propagatedBuildInputs = [
      xo-indentlog

      vulkan-headers
      vulkan-loader
      SDL2.dev
      xorg.libX11.dev  # e.g. sdl2 needs X11/xlib.h

      glew
      libGL
    ];
  })
