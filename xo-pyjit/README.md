# python bindings for llvm JIT for EGAD (xo-pyjit)

## Links

- [cheatsheet for pyobject<->c++ conversion](https://github.com/pybind/pybind11/issues/1201)

## Getting Started

### Build + install `xo-cmake` dependency

- [github/Rconybea/xo-cmake](https://github.com/Rconybea/xo-cmake)

Installs a few cmake ingredients,  along with a build assistant `xo-build` for XO projects such as this one.

### build + install other necessary XO dependencies
```
$ xo-build --clone --configure --build --install xo-indentlog
$ xo-build --clone --configure --build --install xo-refnct
$ xo-build --clone --configure --build --install xo-subsys
$ xo-build --clone --configure --build --install xo-reflect
$ xo-build --clone --configure --build --install xo-expression
$ xo-build --clone --configure --build --install xo-jit
$ xo-build --clone --configure --build --install xo-pyutil
$ xo-build --clone --configure --build --install xo-pyexpression
```
note: can use `xo-build -n` to dry-run here

### copy `xo-pyjit` repository locally
```
$ xo-build --clone xo-pyjit
```

or equivalently
```
$ git clone git@github.com:Rconybea/xo-pyjit.git
```

### build + install xo-pyjit
```
$ xo-build --configure --build --install xo-pyjit
```

or equivalently:

```
$ PREFIX=/usr/local         # or preferred install location
$ cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -S xo-pyjit -B xo-pyjit/.build
$ cmake --build xo-pyjit/.build -j
$ cmake --install xo-pyjit/.build
```
(also see .github/workflows/main.yml)

## Examples

Assumes `xo-pyjit` installed to `~/local2/lib`,
i.e. built with `PREFIX=~/local2`.
```
PYTHONPATH=~/local2/lib:$PYTHONPATH python
>>> from xo_pyreflect import *
>>> from xo_pyjit import *
>>> from xo_pyexpression import *
```

create a jit from within python
```
>>> mp=MachPipeline.make()
>>> mp.dump_execution_sesion()
JITDylib "<main>" (ES: 0x0000000000446ee0, State = Open)
Link order: [ ("<main>", MatchAllSymbols) ]
Symbol table:
```

build an AST from within python
```
>>> f64_t=TypeDescr.lookup_by_name('double')
>>> x=make_var('x',f64_t)                  # "x" a variable (context not yet known)
>>> f1=make_sin_pm()                 # "sin()"
>>> c1=make_apply(f1,[x])            # "sin(x)"
>>> f2=make_cos_pm()                 # "cos()"
>>> c2=make_apply(f2,[c1])           # "cos(sin(x))"
>>> lm=make_lambda('foo', [x], c2)   # "def foo(x): cos(sin(x))"
>>> lm
<Lambda :name foo :argv [x] :body <Apply :fn <Primitive :name cos :type "double (*)(double)" :value 1> :argv "[<Apply :fn <Primitive :name sin :type \"double (*)(double)\" :value 1> :argv \"[<Variable :name x>]\">]">>
```

generate llvm IR for our AST
```
>>> code=mp.codegen(lm)
>>> print(code.print())
define double @foo(double %x) {
entry:
  %calltmp = call double @sin(double %x)
  %calltmp1 = call double @cos(double %calltmp)
  ret double %calltmp1
}
```

generate machine code for our AST,  lookup compiled function so we can invoke it directly
```
>>> mp.machgen_current_module()
>>> mp.dump_execution_session()
JITDylib "<main>" (ES: 0x0000000000446ee0, State = Open)
Link order: [ ("<main>", MatchAllSymbols) ]
Symbol table:
    "foo": <not resolved>  [Callable] Never-Searched (Materializer 0x646fe0, xojit)
>>> fn=mp.lookup_fn('double (*)(double, double)', 'foo')

>>> mp.dump_execution_session()
JITDylib "<main>" (ES: 0x0000000000446ee0, State = Open)
Link order: [ ("<main>", MatchAllSymbols) ]
Symbol table:
    "cos": 0x7ffff7926670 [Data] Ready
    "foo": 0x7fffee2b6000 [Callable] Ready
    "sin": 0x7ffff7925e50 [Data] Ready
```

invoke just-compiled code!
```
>>> fn(22)
0.999960827417674
```

## Development

### use from build tree

Limited utility: requires that supporting libraries (e.g. `xo_pyexpression`) appear in PYTHONPATH
```
$ cd xo-pyjit/.build/src/pyjit
$ python
>>> import xo_pyjit
```

### build for unit test coverage
```
$ cd xo-pyexpression
$ cmake -DCMAKE_BUILD_TYPE=coverage -DENABLE_TESTING=on -S . -B .build-ccov
$ cmake --build .build-ccov -j
```

### LSP (language server) support

LSP looks for compile commands in the root of the source tree;
while Cmake creates them in the root of its build directory.

```
$ cd xo-pyexpression
$ ln -s .build/compile_commands.json  # supply compile commands to LSP
```

### display cmake variables

- `-L` list variables
- `-A` include 'advanced' variables
- `-H` include help text

```
$ cd xo-pyjit/.build
$ cmake -LAH
```
