{
  lib,
  stdenv,
  pkg-config,
  cmake,
  cairo,
  fribidi,
  hyprutils,
  hyprwayland-scanner,
  libdatrie,
  libGL,
  libjpeg,
  libselinux,
  libsepol,
  libthai,
  libxkbcommon,
  pango,
  pcre,
  pcre2,
  utillinux,
  wayland,
  wayland-protocols,
  wayland-scanner,
  xorg,
  debug ? false,
  version ? "git",
}:
stdenv.mkDerivation {
  pname = "hyprpicker" + lib.optionalString debug "-debug";
  inherit version;

  src = ../.;

  cmakeBuildType =
    if debug
    then "Debug"
    else "Release";

  nativeBuildInputs = [
    cmake
    hyprwayland-scanner
    pkg-config
  ];

  buildInputs = [
    cairo
    fribidi
    hyprutils
    libdatrie
    libGL
    libjpeg
    libselinux
    libsepol
    libthai
    libxkbcommon
    pango
    pcre
    pcre2
    utillinux
    wayland
    wayland-protocols
    wayland-scanner
    xorg.libXdmcp
  ];

  outputs = [
    "out"
    "man"
  ];

  meta = with lib; {
    homepage = "https://github.com/hyprwm/hyprpicker";
    description = "A wlroots-compatible Wayland color picker that does not suck";
    license = licenses.bsd3;
    platforms = platforms.linux;
    mainProgram = "hyprpicker";
  };
}
