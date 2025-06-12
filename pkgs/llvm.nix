{
  lib,

  stdenv, cmake, python3,

  enableSharedLibraries ? !stdenv.hostPlatform.isStatic,
  enableManpages ? false,
  enablePolly ? true,

  libxml2, libffi, libedit, ncurses, zlib,
  libpfm, valgrind,

  doCheck ? false,

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

      # don't try multiple outputs, hard to disentangle
      outputs = [ "out" ];

      nativeBuildInputs = [ cmake python3
                          ] ++ lib.optional enableManpages [ python3.pkgs.sphinx ];

      #NIX_CFLAGS_LINK = "-fuse-ld=gold"; # or -fuse-ld=lld

      buildInputs = [ libxml2
                      libffi
                      zlib
                      ncurses
                      libedit
                    ] ++ lib.optionals stdenv.hostPlatform.isLinux [
                      libpfm # performance monitoring
                      valgrind # debugging
                    ];

      propagatedBuildInputs = [ ncurses zlib ];

      inherit doCheck;

      cmakeFlags = [
        # build type and optimization
        "-DCMAKE_BUILD_TYPE=Release"
        "-DLLVM_OPTIMIZED_TABLEGEN=ON"

        "-DLLVM_INSTALL_UTILS=ON"
        "-DLLVM_BUILD_TESTS=${if doCheck then "ON" else "OFF"}"

        # gnuinstalldirs
        "-DCMAKE_INSTALL_INCLUDEDIR=${placeholder "out"}/include"
        # -DCMAKE_INSTALL_LIBDIR, -DCMAKE_INSTALL_LIBEXECDIR don't seem to have any effect for llvm build


        # target architectures
        "-DLLVM_TARGETS_TO_BUILD=${defaultTargets}"

        # library settings
        "-DLLVM_ENABLE_RTTI=ON"
        "-DLLVM_ENABLE_EH=ON"    # exception handling
        "-DLLVM_ENABLE_FFI=ON"
        "-DLLVM_ENABLE_ZLIB=ON"
        "-DLLVM_ENABLE_TERMINFO=ON"
        "-DLLVM_ENABLE_DUMP=ON"

        # testing/benchmarks
        "-DLLVM_BUILD_TESTS=${if doCheck then "ON" else "OFF"}"

        # installation paths using GNUInstallDirs
        "-DLLVM_INSTALL_PACKAGE_DIR=${placeholder "out"}/lib/cmake/llvm"  # will contain refs back to out
        "-DLLVM_INSTALL_BINUTILS_SYMLINKS=OFF"

        # host configuration
        "-DLLVM_HOST_TRIPLE=${stdenv.hostPlatform.config}"
        "-DLLVM_DEFAULT_TARGET_TRIPLE=${stdenv.hostPlatform.config}"

        # polly optimizer
        (lib.optionalString enablePolly "-DLLVM_ENABLE_PROJECT=polly")
      ] ++ lib.optionals enableSharedLibraries [
        # shared libraries
        "-DLLVM_BUILD_LLVM_DYLIB=${lib.boolToString enableSharedLibraries}"
        "-DLLVM_LINK_LLVM_DYLIB=${lib.boolToString enableSharedLibraries}"
      ] ++ lib.optionals enableManpages [
        # documentation
        "-DLLVM_BUILD_DOCS=ON"
        "-DLLVM_ENABLE_SPHINX=ON"
        "-DSPHINX_OUTPUT_MAN=ON"
        "-DSPHINX_OUTPUT_HTML=OFF"
        "-DSPHINX_WARNINGS_AS_ERRORS=OFF"
      ];

      buildFlags = [
        "VERBOSE=1"
        "-j 22"
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

        echo debug=[$debug]
        mkdir -p $debug
        touch $debug/rolandwashere
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
