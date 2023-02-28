# precompiled options

```
~ + <key> <value>
```


```
# tells the compiler that we want a uninitialized data type to use that the compiler can initialize for us.
~interlink <type> <name> <option = mutable | strict | solid> => <wanted value>
```

```
# Snips in the value of <name> into any occurance where <name> can be found, in compile time.
~define <name> <value>
```

```
# Enables a compiler flag.
~cflag <flag> <1/0>

# for example(game.em):

~cflag GAME_LIB_ENABLE 1

```
```
~cflag 
```