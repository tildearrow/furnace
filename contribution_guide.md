# Contribution guide

You can create pull requests as usual, however, you may be given write access to the repo which means that you can work directly in **this** repo. You will be given an access after at least 3 merged pull requests where at least one of them is of significant effort.

Usual pull requests can be as short as one line, or even small fixes in this one line. Significant effort pull request, however, means a substantial change made to the program. For example: adding new chip, refactoring major feature like macros or instrument editor, structuring the big piece of code, implementing new relatively complex feature (e.g. local wavetables — per-instrument wavetables).

Please, before you start working on some big pull request, ask others (preferably on Discord server), maybe somebody is already working on it.

## Work with write access

When you are given write access to the repo, you can work directly in it. However, you would still need to create pull request, but this time it would be different.

First, when you want to start working on some big change, no matter what it is, you also need to ask others to avoid the situation of two persons independently working on the same thing.

You will need to create your own branch there, in `LTVA1/Furnace` repo. It's convenient to name the branch like `user/change_description`, so e.g. when you are `BaaFur2000` and you refactor module saving/loading code, you name the branch `BaaFur2000/refactor_module_saveload`. Multiple persons can work on the same task, but it may be better to separate it into two branches anyway to avoid collisions. After work is done all the individual branches can be merged into some selected one, which would be the branch that gets merged in pull request.

Pull request name must hold a short description of the change, as usual pull request should do. It would be nice to explain the change in more detail in the conversation.

Generally you can merge such pull request immediately after creating it, but it would be nice to see if automatic build fails. When builds pass, you can merge the request all by yourself.

Why such a system is needed is because you can revert all the changes you did by just reverting one single pull request commit. It allows to quickly exclude the code which e.g. is prone to crashes from master branch, rework it until it does not crash anymore, and then merge again.
