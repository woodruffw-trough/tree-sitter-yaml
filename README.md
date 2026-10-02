# tree-sitter-yaml

[![CI][ci]](https://github.com/woodruffw-trough/tree-sitter-yaml/actions)
[![discord][discord]](https://discord.gg/w7nTvsVJhm)
[![matrix][matrix]](https://matrix.to/#/#tree-sitter-chat:matrix.org)
[![crates][crates]](https://crates.io/crates/woodruffw-trough-tree-sitter-yaml)

A tree-sitter parser for YAML files. This fork provides Rust bindings only.

## Rust

Add `woodruffw-trough-tree-sitter-yaml` to your Cargo dependencies and use
`woodruffw_trough_tree_sitter_yaml::LANGUAGE` to load the grammar.

## Development

Install Rust, a C compiler, Make, Python 3.9 or newer, and the pinned Tree-sitter CLI:

```sh
cargo install tree-sitter-cli --version 0.27.0 --locked
make generate
make test
```

`make generate` uses Tree-sitter's bundled QuickJS runtime to evaluate the YAML
and scalar-schema grammars. A Python script then extracts the scalar-schema C
tables from the generated lexers. It uses only the Python standard library.

`make test` runs the scanner tests, Rust unit tests and doctests, parser corpus
tests, and highlight tests. Cargo builds use the checked-in C sources.

## References

- [YAML version 1.2](https://yaml.org/spec/1.2.2/)

[ci]: https://img.shields.io/github/actions/workflow/status/woodruffw-trough/tree-sitter-yaml/ci.yml?logo=github&label=CI
[discord]: https://img.shields.io/discord/1063097320771698699?logo=discord&label=discord
[matrix]: https://img.shields.io/matrix/tree-sitter-chat%3Amatrix.org?logo=matrix&label=matrix
[crates]: https://img.shields.io/crates/v/woodruffw-trough-tree-sitter-yaml?logo=rust
