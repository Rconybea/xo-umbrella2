# xo-object2

# Example

```
# xo-python

import gc, json
import xo.facet as f
import xo.object2 as o
import xo.stringtable2 as st

fcx = f.configure_all()
stcx = st.configure(st.Stringtable2Config(), fcx)
fw = f.AllocFlywheel.make_default_app(fcx)

frame = lambda: json.loads(o.flyweheel_frame(fw)

print(json.dumps(frame()["pools"], indent=1))
```
