# Hyprgrab
A simple program to take screenshots and record the screen on Hyprland.

```
Usage: hyprgrab screenshot|screencast [options]

Hyprgrab is a small utility program to easily capture the screen in Hyprland.

It will use hyprctl and slurp for region selection, then grim or wl-screenrec
to take the screenshot or record the screen. The result file will be stored
with the name 'hyprgrab_{shot|cast}_{time}.{ext}' in the chosen folder.

When recording, a new terminal window will be opened with the title
"hyprgrab-recorder". I recomend adding some rules to make the window float
in your Hyprland config like this:
  windowrulev2 = float, title: ^(hyprgrab-recorder)$

Options:
  -h        show this help message
  -m        one of: output, window, region
  -o        directory in which to save result. ~/{Pictures|Videos} by default
  -s        seconds to sleep before taking the screenshot/recording
  -t        command to open a named terminal when recording. Default is
            'kitty --title "hyprgrab-recorder"'

Modes:
  output:   take screenshot of an entire monitor (default)
  window:   take screenshot of an open window
  region:   take screens
```

