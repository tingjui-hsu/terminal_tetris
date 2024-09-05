# Tetris in terminal

## installation

```
$ git clone https://github.com/tingjui-hsu/terminal_tetris.git
$ make
```

## usage

```
$ tetris
```

press `7`/`9`/`5` to move
press `8` to turn
press `4` increase speed
press `1` to show/hide next
press `0` to show/hide guide

and you can modify these macro to change keys.

```c
#define KEY_LEFT   '7'
#define KEY_RIGHT  '9'
#define KEY_ROTATE '8'
#define KEY_DOWN   '5'
#define KEY_ACC    '4'
#define KEY_NEXT   '1'
#define KEY_GUIDE  '0'
```
