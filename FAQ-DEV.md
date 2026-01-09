Error like this:

```
/home/roland/proj/xo-umbrella2-claude1/./xo-facet/include/xo/facet/OObject.hpp:63:19:   required from ‘struct xo::facet::OObject<xo::mm::AGCObject, xo::scm::DList>’
   63 |             using ISpecific = FacetImplType<AFacet, DRepr>;
      |                   ^~~~~~~~~
/home/roland/proj/xo-umbrella2-claude1/./xo-gc/include/xo/gc/detail/RGCObject.hpp:15:16:   required from ‘struct xo::mm::RGCObject<xo::facet::OObject<xo::mm::AGCObject, xo::scm::DList> >’
   15 |         struct RGCObject : public Object {
      |                ^~~~~~~~~
/home/roland/proj/xo-umbrella2-claude1/./xo-facet/include/xo/facet/obj.hpp:49:16:   required from ‘struct xo::facet::obj<xo::mm::AGCObject, xo::scm::DList>’
   49 |         struct obj : public RoutingType<AFacet, OObject<AFacet, DRepr>> {
      |                ^~~
/home/roland/proj/xo-umbrella2-claude1/xo-object2/utest/Printable.test.cpp:81:57:   required from here
   81 |                 auto l0_o = with_facet<AGCObject>::mkobj(l0);
      |                             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~
/home/roland/proj/xo-umbrella2-claude1/./xo-facet/include/xo/facet/facet_implementation.hpp:97:15: error: no type named ‘ImplType’ in ‘struct xo::facet::FacetImplementation<xo::mm::AGCObject, xo::scm::DList>’
   97 |         using FacetImplType = FacetImplementation<AFacet, DRepr>::ImplType;
      |               ^~~~~~~~~~~~~
```

means implementation `xo::mm::AGCObject` for `xo::scm::DList` is not availabel to compiler.
Either missing implementation or missing header.

In example implementation would be in a file `IGCObject_DList.hpp`
