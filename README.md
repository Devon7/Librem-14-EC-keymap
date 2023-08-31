# System76 EC

System76 EC is a GPLv3 licensed embedded controller firmware for System76
laptops.

## Devonian custom keyboard layout for Librem-14:

To swap Ctrl/Fn & CapsLock/Backspace:

```
make BOARD=purism/librem_14 VERSION=1.13-devon_2023-03-22 KEYMAP=devon EXTRA_CFLAGS=-DDEVON
```

This builds a Librem-14 EC with MacBook/ThinkPad style Fn key
and Knight/LispMachine/SpaceCadet style Backspace/Rubout key.

The docs below fail to mention extra work needed to move Fn:

```
git diff
```

just a quick hack but I like it.

		Peace
			--Devon

P.S.  At a glance, the extra Fn Esc machinery seems awkward
but I have resisted the temptation to improve that code.

## Documentation

- [Supported embedded controllers](./doc/controllers.md)
- [Flashing firmware](./doc/flashing.md)
- [Debugging](./doc/debugging.md)
- [Creating a custom keyboard layout](./doc/keyboard-layout-customization.md)
- [Adding a new board](./doc/adding-a-new-board.md)

## Dependencies

Install dependencies using the provided script:

```
./scripts/deps.sh
```
