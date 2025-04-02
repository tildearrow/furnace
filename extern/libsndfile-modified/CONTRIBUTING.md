# Contributing

## Submitting Issues

* If your issue is that libsndfile is not able to or is incorrectly reading one
  of your files, please include the output of the `sndfile-info` program run
  against the file.
* If you are writing a program that uses libsndfile and you think there is a bug
  in libsndfile, reduce your program to the minimal example, make sure you compile
  it with warnings on (for GCC I would recommend at least `-Wall -Wextra`) and that
  your program is warning free, and that is is error free when run under Valgrind
  or compiled with AddressSanitizer.

## Submitting Patches

* Patches should pass all existing tests
* Patches should pass all pre-commit hook tests.
* Patches should always be submitted via a either Github "pull request" or a
  via emailed patches created using "git format-patch".
* Patches for new features should include tests and documentation.
* Commit messages should follow the ["How to Write a Git Commit Message"](https://chris.beams.io/posts/git-commit/) guide:
  1. Separate subject from body with a blank line
  2. Limit the subject line to 50 characters
  3. Capitalize the subject line
  4. Do not end the subject line with a period
  5. Use the imperative mood in the subject line
  6. Wrap the body at 72 characters
  7. Use the body to explain what and why vs. how

  Additional rule: the commit message may contain a prefix. The prefix must
  contain the name of the feature or source file related to the commit and must
  end with a colon followed by the message body.

  Examples of good commit messages:
  1. Fix typo
  2. Update CHANGELOG.md
  3. Add ACT file format support
  4. ogg_vorbis: Fix granule position when seeking Vorbis streams

  Examples of bad commit messages:
  1. Fixed bug (rule 5)
  2. update docs (rule 3)
  3. Add very cool feature. (rule 4)

* Patches to fix bugs should either pass all tests, or modify the tests in some
  sane way.
* When a new feature is added for a particular file format and that feature
  makes sense for other formats, then it should also be implemented for one
  or two of the other formats.
