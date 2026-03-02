{
  lib,
  inputs,
  self,
  ...
}:
let
  mkDate =
    longDate:
    (lib.concatStringsSep "-" [
      (builtins.substring 0 4 longDate)
      (builtins.substring 4 2 longDate)
      (builtins.substring 6 2 longDate)
    ]);
  version = lib.removeSuffix "\n" (builtins.readFile ../VERSION);
in
{
  default = self.overlays.hyprpicker;
  hyprpicker-with-deps = lib.composeManyExtensions [
    inputs.hyprutils.overlays.default
    inputs.hyprwayland-scanner.overlays.default
    self.overlays.hyprpicker
  ];

  hyprpicker = final: prev: {
    hyprpicker = prev.callPackage ./default.nix {
      stdenv = prev.gcc15Stdenv;
      version =
        version
        + "+date="
        + (mkDate (self.lastModifiedDate or "19700101"))
        + "_"
        + (self.shortRev or "dirty");
    };
    hyprpicker-debug = final.hyprpicker.override { debug = true; };
  };
}
