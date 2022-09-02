# hyprpicker

A wlroots-compatible Wayland color picker that does not suck.

![hyprpickerShort](https://user-images.githubusercontent.com/43317083/188224867-7d77a3b3-0a66-488c-8019-39b00060ab42.gif)

# Usage

Launch it. Click. That's it.

## Options

`--format [fmt]` to specify the output format (`hex`, `rgb`)

`--no-fancy` disables the "fancy" (aka. colored) outputting

# Building

It's also on the AUR as `hyprpicker-git`

`make all`

the output binary is in `./build/hyprpicker`

# Caveats

"Freezes" your displays when picking the color.
