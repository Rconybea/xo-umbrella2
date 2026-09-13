# xo-pystringtable2
python bindings for xo-stringtable2


## Example

```
$ xo-python
>>> import xo.facet as f
>>> import xo.stringtable2 as st
>>> fcx = f.configure_all()
>>> stcx = st.configure(st.Stringtable2Config(), fcx)
>>> fw = f.AllocFlywheel.make_default_app(fcx)
>>> s = st.String.make(fw, "hello")
>>> s.value()
hello

```
