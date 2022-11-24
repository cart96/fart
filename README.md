<div align="center">

# Fart

True brainfuck assembler that generates binary from source for DOS.

![FART IN DOS](./assets/action.png)

(assembled brainfuck runs on DOSBox)

</div>

## Informations

- Written in C99.
- Can handle maximum 4096 loop. (you can modify it from source.)
- Only has 4096 cell. (you can modify it from source.)
- Designed for 8086.

## Downloading

You can download pre-built fart binaries from github releases.

## Compiling

### Automatic Installation

Copy and paste following command to your terminal and it will compile from source and add output binary to /usr/bin folder.

```
curl -fsSL https://raw.githubusercontent.com/cart96/fart/main/install.sh | sh
```

### Manual Installation

Go to `src/fart/scripts` and run `build.bat` or `build.sh` depend on your OS.

## Usage

- `<...>` = Required
- `[...]` = Optional


You can use fart like `fart <input file> [output file]` this in command-line. If you don't pass output file, it will write output to `A.COM`.

- `fart e.bf euler.com`
- `fart hello.b`

## Special Thanks

- Thanks [Gülce](https://github.com/gulje) for helping me while debugging ❤️
- Thanks [Skeeto](https://github.com/skeeto/brainfuck-fart) for making a DOS version of fart ❤️

## License

Fart is licensed under the 3-Clause BSD License.
