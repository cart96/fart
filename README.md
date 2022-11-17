<div align="center">

# Fart

True brainfuck assembler that generates binary from source for DOS.

![FART IN ACTION](./assets/action.png)

(assembled brainfuck runs on DOSBox)

![FART DOS](https://i.imgur.com/0iNYFNT.png)

(fart edited for DOS)

</div>

## Informations

- Written in C99.
- Can handle maximum 2048 loop. (you can modify it from source.)
- Only has 4096 cell. (you can modify it from source.)
- Designed for 8086.

## Compiling

Go to `src/fart` and run `build.bat` or `build.sh` depend on your OS.

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
