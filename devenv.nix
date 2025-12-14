{ pkgs, lib, config, inputs, ... }:

{
  packages = with pkgs; [ git openssl gcc gnumake cmake ];
  enterShell = ''
    git --version # Use packages
  '';
}
