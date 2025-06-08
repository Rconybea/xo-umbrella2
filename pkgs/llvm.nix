{
  lib,

  stdenv, cmake, python3,

  enableSharedLibraries ? !stdenv.hostPlatform.isStatic,

  libxml2, libffi, libedit, ncurses, zlib,

  fetchFromGitHub,
} :

let
  version = "18.1.7";

  src = fetchFromGitHub {
    name = "llvm-${version}-source";
    owner = "llvm";
    repo = "llvm-project";
    rev = "llvmorg-${version}";
    hash = "sha256-qBL/1zh2YFabiPAyHehvzDSDfnwnCvyH6nY/pzG757A=";
  };

  defaultTargets =
    if stdenv.hostPlatform.isAarch64 then "AArch64"
    else if stdenv.hostPlatform.isx86 then "X86"
    else "all";

in {
  # running out of memory during linking!!

  llvm = stdenv.mkDerivation (finalAttrs:
    {
      pname = "llvm-xo";
      inherit version src;

      # only taking llvm subdir from upstream repo
      sourceRoot = "${src.name}/llvm";

      outputs = [ "out" ];

      nativeBuildInputs = [ cmake python3 ];

      #NIX_CFLAGS_LINK = "-Wl,--no-keep-memory, -Wl,--reduce-memory-overheads";
      #NIX_CFLAGS_LINK = "-fuse-ld=gold"; # or -fuse-ld=lld

      buildInputs = [ libxml2
                      libffi
                      zlib
                      ncurses
                      libedit
      ];

      propagatedBuildInputs = [ ncurses zlib ];

      cmakeFlags = [
        "-DCMAKE_BUILD_TYPE=Release"

        "-DLLVM_TARGETS_TO_BUILD=${defaultTargets}"

        "-DLLVM_BUILD_LLVM_DYLIB=${lib.boolToString enableSharedLibraries}"
        "-DLLVM_LINK_LLVM_DYLIB=${lib.boolToString enableSharedLibraries}"

        # host configuration
        "-DLLVM_INSTALL_CMAKE_DIR=${placeholder "bin"}/lib/cmake/llvm"
        "-DLLVM_INSTALL_BINUTILS_SYMLINKS=OFF"

      ];

      buildFlags = [
        "VERBOSE=1"
        "-j 12"
      ];

      # running out of memory!
      enableParallelBuilding = true;

      # for llvm keep debug symbols
      separateDebugInfo = stdenv.isLinux;

      postInstall = ''
        # create compatibility symlinks
        mkdir -p $out/bin
        for tool in llvm-config llvm-nm llvm-objdump llvm-readobj llvm-ar; do
          ln -sf $out/bin/$tool $out/bin/${stdenv.hostPlatform.config}-$tool
        done

#        # dev related files to llvm.dev
#        moveToOutput "lib/cmake" "$dev"
#        moveToOutput "include" "$dev"
#        moveToOutput "lib/*.a" "$dev"

#        # move any shared libraries to llvm.lib
#        mkdir -p $lib/lib
#        mv $out/lib/*.so* $lib/lib/ || true
      '';

      passthru = {
        inherit src;
        isClang = false;
        hardeningUnsupportedFlags = [ "fortify3" ];
      };

      meta = with lib; {
        description = "A collection of modular and reusable compiler and toolchain technologies";
        homepage = "https://llvm.org/";
        license = lib.licenses.ncsa;
        maintainers = with maintainers; [];
        platforms = platforms.all;
        broken = stdenv.hostPlatform.is32bit;
      };
    });
}
