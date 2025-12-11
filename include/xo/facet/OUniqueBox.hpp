/** @file OUniqueBox.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

namespace xo {
    namespace facet {
        /**
         *  Uniquely-owned instance with runtime polymorphism.
         *
         *  Reminder that in the facet object model we expect
         *  objects to be transient.
         *

         *
         *  Unlike OUniqueBox<AInterface, ..> can use for variant data
         *  without additional overhead. Tradeoff is that avoiding such
         *  overhead excludes std::unique_ptr.
         *
         *  We're going to instead rely on AInterface providing a destruct_data() method,
         *  so in practice get the deleter from interface state.
         *
         *  Possibly means we need all abstract interfaces to share a common base
         *
         *  Remarks:
         *  - when @tparam Data is supplied
         **/
        template <typename AInterface, typename Data = DOpaquePlaceholder>
        struct OUniqueBox {
            using AbstractInterface = AInterface;
            using ISpecific = ISpecificFor<AInterface, Data>::ImplType;
            /* note: Data can be void here */
            using DataType = Data;
            using DataBox = Data*;

            explicit OUniqueBox() {}
            /* unsatisfactory b/c doesn't enforce that @p d is heap-allocated */
            explicit OUniqueBox(DataBox d) : data_{std::move(d)} {}

            ~OUniqueBox() {
                if (data_ != nullptr) {
                    this->iface()->destruct_data(data_);
                    delete data_;
                    this->data_ = nullptr;
                }
            }

            const AInterface * iface() const
                   requires std::is_same_v<Data, DOpaquePlaceholder>
            {
                return std::launder(&iface_);
            }

            const AInterface * iface() const
                  requires (!std::is_same_v<Data, DOpaquePlaceholder>)
            {
                return &iface_;
            }

            /** note: would prefer this to be constexpr, but not simple asof gcc 14.3 **/
            static bool _valid;

            /** note: load-bearing for routing classes such as RComplex<OUniqueBox> **/
            Data * data() const { return data_; }

            ISpecific iface_;
            DataBox data_ = nullptr;
        };


    }
} /*namespace xo*/

/* end OUniqueBox.hpp */
