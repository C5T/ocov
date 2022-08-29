# `ocov`

The `ocov` tool, short for `OPA Coverage Report`, colors into green and red the covered / not covered lines of the Rego policies, as output by `opa test --coverage`.

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

## Use

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
ocov --version  # Or ocov -v
```

And:

```
ocov --help
```
