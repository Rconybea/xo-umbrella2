# xo-alloc2 -- arena allocator and incremental garbage collector

# Relative to xo-alloc:

1. keep interface and data separate.
   1a. `Representation` classes. Entirely passive; strictly no methods.
       motivation: data doesn't carry any linker-dependency baggage;
       it's just layout.

       example:
         struct RPolar { double arg; double mag; };
         struct RRect { double x; double y; };

   1b. `Interface` classes. These have abstract methods only.
       motivation: for runtime polymorphism, specify interface
       without assuming anything about data layout.
       Methods in an interface will take opaque data pointer
       as first argument.

       example:
         struct IComplex {
            using repr_type = void;

            virtual double xcoord(void * repr) const = 0;
            virtual double ycoord(void * repr) const = 0;
            virtual double magnitude(void * repr) const = 0;
            virtual double argument(void * repr) const = 0;
         };

   1c. `Implementation` classes. Implement a specific interface (as in 1b)
       for a specific data representation (as in 1a).
       All methods will be `final override`.
       Methods in implementation, since they inherit an interface,
       wil have opaque data pointer as their first argument.
       They will downcast this pointer to specific target representation.

       example:
```
         struct Complex_Rect {
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

         struct Complex_Polar {
           using repr_type = RPolar;

           // implement IComplex for RPolar
           ..similar..
         };
```

   1d. `Object` classes. Pair implementation and interface.
       May use smart pointer here to express strategy for managing
       memory used for representation. Don't expect to need this for
       interfaces, since interface content entirely known at compile time.

       example:
         // borrowed
         struct _Complex_Rect : public Complex_Rect {
           bp<RRect> repr;   // naked pointer
         };

         struct _Complex_Polar : public Complex_Polar {
           bp<RPolar> repr;
         };

         // unique
         struct _Complex_Rect : public Complex_Rect {
           up<RRect> repr;  // unique_ptr
         };

         ..

        Can do this generically.

          template <typename Iface,
                    typename Repr = typename Iface::repr_type>
          struct bxp : public Iface {
            bp<Repr> data_;
          };

          using t1 = bxp<Complex_Rect>;
          using t2 = bxp<Complex_Polar>;

        etc.

        Then to invoke a method (compile-time polymorphism)

          bxp<Complex_Rect> obj;
          obj.xcoord(obj.data_);  // obj.xcoord()

        Or for runtime polymorphism

          bxp<IComplex> obj;
          obj.xcoord(obj.data_);  // obj.xcoord()

   1e. Runtime polymorphism

       Observe that bxp<Complex_Rect> and bxp<Complex_Polar> have the same
       top-level representation.

       - Both have iface member that inherits IComplex,
       - both have data pointer compatible with their respective iface member

       Can have common representation for runtime polymorphism

       - `bxp<Complex_Rect>` and `bxp<Complex_Polar>` have the same size
         and compatible representation.
       - both inherit `IComplex`

       - safe to reinterpret cast to

   2. Remarks
      - shared pattern with pimpl idiom,
        except impl isn't private
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
