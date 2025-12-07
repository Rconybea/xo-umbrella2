# xo-alloc2 -- arena allocator and incremental garbage collector

# Relative to xo-alloc:

1. keep interface and data separate.
   1a. *Representation* or *Data* classes. Entirely passive; strictly no methods.
       motivation: data doesn't carry any linker-dependency baggage;
       it's just layout.

       example:
```
         struct DPolar { double arg; double mag; };
         struct DRRect { double x; double y; };
```

   1b. `Interface` classes. These have abstract methods only.
       motivation: for runtime polymorphism, specify interface
       without assuming anything about data layout.
       Methods in an interface will take opaque data pointer
       as first argument.

       example:
```
         struct AComplex {
            using repr_type = void;

            virtual double xcoord(void * repr) const = 0;
            virtual double ycoord(void * repr) const = 0;
            virtual double magnitude(void * repr) const = 0;
            virtual double argument(void * repr) const = 0;
         };
```

   1c. `Implementation` classes. Implement a specific interface (as in 1b)
       for a specific data representation (as in 1a).
       All methods will be `final override`.
       Methods in implementation, since they inherit an interface,
       wil have opaque data pointer as their first argument.
       They will downcast this pointer to specific target representation.

       example:
```
         struct IComplex_Rect : public AComplex {
           using repr_type = RRect;

           double _xcoord(RRect * repr) const { return repr->x; }
           double _ycoord(RRect * repr) const { return repr->y; }
           double _magnitude(RRect * repr) const {
             double x = repr->x;
             double y = repr->y;
             return ::sqrt(x*x + y*y);
           }
           double _argument(RRect * repr) const {
             double tan = repr->x / repr->y;
             return ::arctan(tan);
           }

           // implement IComplex for RRect
           double xcoord(void * repr) const final override {
             return _xcoord((RRect*)repr);
           }
           double ycoord(void * repr) const final override;
           double magnitude(void * repr) const final override;
           double argument(void * repr) const final override;
         };

         struct IComplex_Polar : public AComplex {
           using repr_type = RPolar;

           // implement IComplex for RPolar
           ..similar..
         };
```

       Here `IComplex_Rect` and `IComplex_Polar` are constructible.
       They're concrete in the sense that they expect a specific representation
       (`IComplex_Rect::repr_type`, `IComplex_Polar::repr_type` respectively).

   1d. `Object` classes. Pair implementation and interface.
       May use smart pointer here to express strategy for managing
       memory used for representation. Don't expect to need this for
       interfaces, since interface content entirely known at compile time.

       example:
```
         // borrowed
         struct OComplex_Rect : public IComplex_Rect {
           DRect * data() const { return data_; }

           bp<DRect> data_;   // naked pointer
         };

         struct OComplex_Polar : public IComplex_Polar {
           DPolar * data() const { return data_; }

           bp<DPolar> data_;
         };

         // unique
         struct OComplex_Rect : public IComplex_Rect {
           DRect * data() const { return data_; }

           up<DRect> data_;  // unique_ptr
         };

         ..
```
        Can do this generically.

```
          // in bx: 'b' short for 'borrowed' as in unowned.
          //        'x' just to distinguish from 'pointer'.
          //
          template <typename Iface,
                    typename Repr = typename Iface::repr_type>
          struct bx : public Iface {
            explicit bx(Repr * data) : data_{data} {}
            Repr * data() const { return data_; }

            bp<Repr> data_;
          };

          DRect z1_data{1.0, -1.0};
          bx<IComplex_Rect> z1{&z1_data};

          DPolar z2_data{sqrt(2.0), pi * 8/7};
          bx<IComplex_Polar> z2{&z2_data};
```
        Then to invoke a method (compile-time polymorphism)
```
          z1._xcoord(z1.data());
```

   1e. Runtime polymorphism
       Observe that bxp<Complex_Rect> and bxp<Complex_Polar> have the same
       top-level representation.
       - Both have iface member that inherits IComplex,
       - both have data pointer compatible with their respective iface member
       Can have common representation for runtime polymorphism
       - `bxp<Complex_Rect>` and `bxp<Complex_Polar>` have the same size
         and compatible representation.
       - both inherit `IComplex`
       - safe to reinterpret cast
```
         // type-erased (placeholder, never used)
         struct IComplex_Any : public AComplex {
           using repr_type = void;

           double xcoord(void * repr) const final override { assert(false); return 0.0; }
         };

         bx<IComplex_Rect> z1 = ...;
         bx<IComplex_Any> z1_any = reinterpret_cast<IComplex_Any>(z1);
```
       Capturing the pattern:
```
        // in abstract interface
        struct AComplex {
          using ErasedIfaceType = IComplex_Any;
          ..
        }

        template<typename Iface,
                 typename Repr = typename Iface::repr_type>
        struct bx : public Iface {
          ..
          operator bx<Iface::typename ErasedIfaceType>() {
            // in particular, overwrites vtable pointer
            return reinterpret_cast<bx<Iface::typename ErasedIfaceType>>(*this);
          }
          ..
        };
        ```

   2. Remarks
      - shared pattern with pimpl idiom,
        except impl isn't private
      - can use the same Data type with an unrelated interface.
        Although lose the automatic assocation
      - can put forwarding methods into object structs,
        though will be boilerplatey.

          struct bxp_ext : public bxp {
            double xcoord() { return iface->xcoord(data); }
            double ycoord() { return iface->ycoord(date); }
            double magnitude() { return iface->magnitude(data); }
            double argument() { return iface->argument(data); }
          };
      - since interface and data are segregated,
        it's easier to devirtualize. Interface pointers are explicit,
        and don't need to be changed to refer to different data.
