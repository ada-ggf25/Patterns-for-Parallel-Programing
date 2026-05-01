# Vendored third-party headers

## doctest.h

Fetched at build time from the upstream release tag to avoid committing a large binary blob. The CMake top-level and CI workflow both run:

```bash
curl -fsSL \
  https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h \
  -o snippets/third_party/doctest.h
```

Version: **v2.4.11**. If you upgrade, update this README and the CI workflow in lockstep.

doctest is MIT-licensed.
