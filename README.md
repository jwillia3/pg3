Plain Graphics Library 3
----------------------------------------------------------------


This is a vector graphics library that is modeled after the
PostScript/PDF imaging model.

# Building

```
./configure
make install
```

or to install it somewhere other than the default (/usr or /usr/local)

```
./configure prefix=$HOME/.local
make install
```

to install to `$HOME/.local`.
Note that you should use `$HOME` as `~` is not expanded by the shell in `~/.local`.
