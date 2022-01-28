# Contributing

contributions to Furnace are welcome!

# Getting ready

log into your Github account, and click the Fork button in the header of the project's page.

then open a terminal and clone your fork:

```
git clone git@github.com:USERNAME/furnace.git
```

(replace `USERNAME` with your username)

# Working

## Code

bug fixes, improvements and several other things accepted.

the coding style is described here:

- indentation: two spaces
- modified 1TBS style:
  - no spaces in function calls
  - spaces between arguments in function declarations
  - no spaces in operations except for `||` and `&&`
  - no space between variable name and assignment
  - space between macro in string literals
  - C++ pointer style: `void* variable` rather than `void *variable`
  - indent switch cases
  - preprocessor directives not intended
  - if macro comprises more than one line, indent

some files (particularly the ones in `src/engine/platform/sound` and `extern/`) don't follow this style.

you don't have to follow this style. I will fix it after I accept your contribution.

## Demo Songs

just put your demo song in `demos/`!

# Finishing

after you've done your modifications, commit the changes and push.
then open your fork on GitHub and send a pull request.

# I don't know how to use Git but I want to contribute with a demo song

you can also contact me directly! [find me here.](https://tildearrow.org/?p=contact)
