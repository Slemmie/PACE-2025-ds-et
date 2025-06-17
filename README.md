# PACE-2025-ds-et
PACE challenge 2025, dominating set, exact track

# Building
To build the solver, make sure the following libraries are present on PATH:

```
libCbcSolver libCbc libClp libCgl libOsiClp libOsi libCoinUtils
```

The solver itself resides in `reducer/`

First, build additional dependencies with:

```sh
cd reducer && make vcdep
```

Next, build the binary itself:

```sh
cd reducer && make -j$(ncproc)
```

The solver expects from STDIN a graph following the PACE 2025 dominating set format. It prints to STDOUT a solution.
