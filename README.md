# `ocov`

Colors `opa test --coverage` reports in the terminal.

## Build

```
git clone --recursive https://github.com/c5t/ocov.git
cd ocov
NDEBUG=1 make .current/ocov
```

## Install

Copy `.current/ocov` somewhere into your `$PATH`. The following command does the job on both Mac and Linux.

```
sudo cp .current/ocov /usr/local/bin/
```

## Usage

Either:

```
opa test ${TEST_ARGS} --coverage | ocov
```

Or:

```
opa test ${TEST_ARGS} --coverage > coverage.json
ocov --input coverage.json
```

There are also:

```
ocov --version  # Or ocov -c
```

And:

```
ocov --help
```
