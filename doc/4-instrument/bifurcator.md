# Bifurcator instrument editor

Bifurcator instrument editor consists of these macros:

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Parametet**: set parameter of logistic map function.
- **Panning (left)**: output level for left channel.
- **Panning (right)**: output level for right channel.
- **Pitch**: fine pitch.
- **Load Value**: changes the current output value.

## audio generation description

Bifurcator uses logistic map iterations for sound generation.
basically it runs the following function over and over:

```
r = (1 + (p / 65536)) * 2
x = (r * x) * (1 - x)
```

where `x` is the current output value and `p` is the "parameter".

by varying the parameter, the value of x may change drastically, producing a variety of sounds.
the higher the parameter, the more "chaos" is present, effectively yielding noise.

the default parameter is `47360`, which results in a square wave.

if the parameter is set to 0, there's no sound at all.
as the parameter approaches 32768, a decaying square wave is produced.
the square wave stops decaying past 32768 and becomes louder until the parameter hits ~47496 (`r = 1 + sqrt(6)`).
a second square wave one octave lower then starts appearing, until the parameter reaches ~51443 (`r ≈ 3.56995`). this is where chaos begins.
anything higher results in a total mess.
however, at ~59914 (`r = 1 + sqrt(8)`) you can hear a 33% pulse wave.

## the importance of loading the value

you must load a value that isn't 0 in order to get sound. otherwise the function will always output 0.
