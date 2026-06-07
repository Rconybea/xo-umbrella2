# python bindings for c++ kalman filter library (xo-kalmanfilter)

## Getting Started

### build + install dependencies

- [github/Rconybea/xo-kalmanfilter](https://github.com/Rconybea/xo-kalmanfilter)
- [github/Rconybea/xo-pyreactor](https://github.com/Rconybea/xo-pyreactor)

### build + install

```
$ cd xo-pykalmanfilter
$ mkdir build
$ cd build
$ INSTALL_PREFIX=/usr/local  # or wherever you prefer, e.g. ~/local
$ cmake \
    -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake \
    -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
$ make
$ make install
```
(also see .github/workflows/main.yml)

## Examples

Assumes `xo-pykalmanfilter` installed to `~/local2/lib`
```
PYTHONPATH=~/local2/lib:$PYTHONPATH python
>>> import xo_pykalmanfilter
>>> dir(xo_pykalmanfilter)
['KalmanFilter', 'KalmanFilterInput', 'KalmanFilterInputToConsole', 'KalmanFilterObservable', 'KalmanFilterSpec', 'KalmanFilterState', 'KalmanFilterStateEventStore', 'KalmanFilterStateExt', 'KalmanFilterStateToConsole', 'KalmanFilterStep', 'KalmanFilterSvc', 'KalmanFilterTransition', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__', 'kf_engine_correct', 'kf_engine_correct1', 'kf_engine_extrapolate', 'kf_engine_gain', 'kf_engine_gain1', 'make_kalman_filter', 'make_kalman_filter_input', 'make_kalman_filter_input_printer', 'make_kalman_filter_state_printer', 'print_matrix', 'print_vector']
>>>
```

## Development

### build for unit test coverage
```
$ cd xo-pykalmanfilter
$ mkdir build-ccov
$ cd build-ccov
$ cmake \
    -DCMAKE_MODULE_PATH=${INSTALL_PREFIX}/share/cmake \
    -DCMAKE_PREFIX_PATH=${INSTALL_PREFIX} \
    -DCODE_COVERAGE=ON \
    -DCMAKE_BUILD_TYPE=Debug ..
```

### LSP (language server) support

LSP looks for compile commands in the root of the source tree;
while Cmake creates them in the root of its build directory.

```
$ cd xo-pykalmanfilter
$ ln -s build/compile_commands.json  # supply compile commands to LSP
```

### display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd xo-pykalmanfilter/build
$ cmake -LAH
```
