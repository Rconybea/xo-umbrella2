/* AppState.cpp */

#include "AppState.hpp"
#include "xo/object/Integer.hpp"
#include "xo/object/List.hpp"

using xo::IObject;
using xo::obj::Integer;
using xo::obj::List;
using xo::gp;

AppState::AppState()
{
    this->gc_ = (GC::make
           (
               {.initial_nursery_z_ = 1024*1024,
                .initial_tenured_z_ = 1024*1024*1024,
                .incr_gc_threshold_ = 16*1024,
                .full_gc_threshold_ = 128*1024,
                .stats_flag_ = true,
                .debug_flag_ = false}));

    Object::mm = gc_.get();

    for (auto & x: gc_root_v_)
        gc_->add_gc_root(reinterpret_cast<IObject **>(x.ptr_address()));

    gc_->disable_gc();
}

std::size_t
AppState::nursery_tospace_scale() const {
    std::size_t N1_to_size = gc_->nursery_before_checkpoint();
    std::size_t N_to_committed = gc_->nursery_to_committed();
    std::size_t N_to_incr_gc_threshold = N1_to_size + gc_->config().incr_gc_threshold_;
    std::size_t N_to_scale = std::max(N_to_committed, N_to_incr_gc_threshold);

    return N_to_scale;
}

std::size_t
AppState::tenured_tospace_scale() const {
    std::size_t T1_to_size = gc_->tenured_before_checkpoint();
    std::size_t T_to_committed = gc_->tenured_to_committed();
    std::size_t T_to_full_gc_threshold = T1_to_size + gc_->config().full_gc_threshold_;
    std::size_t T_to_scale = std::max(T_to_committed, T_to_full_gc_threshold);

    return T_to_scale;
}

GcStateDescription
AppState::snapshot_gc_state() const {
    /** NOTE: this gets invoked before GC gets opportunity to run.
     *        in the event that GC does run, from- and to- spaces will
     *        have been reversed (near beginning of GC phase)
     *
     *        This means that nursery_to_reserved() etc. actually refer to from-space
     *        *during gc*
     **/

    // TOOD: may want to use GC::get_gc_statistics() to replace multiple round trips

    return GcStateDescription(GcGenerationDescription
                              (generation::nursery,
                               "nursery",
                               "N",
                               "incremental",
                               gc_->nursery_polarity(),
                               this->nursery_tospace_scale(),
                               gc_->nursery_before_checkpoint(),
                               gc_->nursery_after_checkpoint(),
                               gc_->nursery_to_reserved(),
                               gc_->nursery_to_committed(),
                               gc_->nursery_before_checkpoint() + gc_->config().incr_gc_threshold_),
                              GcGenerationDescription
                              (generation::tenured,
                               "tenured",
                               "T",
                               "full",
                               gc_->tenured_polarity(),
                               this->tenured_tospace_scale(),
                               gc_->tenured_before_checkpoint(),
                               gc_->tenured_after_checkpoint(),
                               gc_->tenured_to_reserved(),
                               gc_->tenured_to_committed(),
                               gc_->tenured_before_checkpoint() + gc_->config().full_gc_threshold_),

                              gc_->size(),
                              gc_->committed(),
                              gc_->allocated(),
                              gc_->available(),
                              gc_->mlog_size(),
                              gc_->native_gc_statistics().total_promoted_,
                              gc_->native_gc_statistics().n_mutation_
        );
}

void
AppState::generate_random_mutation() {
    if (rng_() % 1000 > (5 * 1000) / 7) {
        /* p=16% integer */
        gc_root_v_[next_root_++] = Integer::make(gc_.get(), next_int_);
    } else if (rng_() % 1000 > (3 * 1000) / 7) {
        /* p=16% cons */
        gp<Object> random_car = gc_root_v_.at(rng_() % gc_root_v_.size());
        if (random_car.is_null())
            random_car = List::nil;

        /* this will always incorporate existing list as tail of new list */
        gp<List> random_cdr = List::from(gc_root_v_[next_root_]);
        if (random_cdr.is_null())
            random_cdr = List::nil;

        gp<List> random_cons = List::cons(random_car, random_cdr);

        gc_root_v_[next_root_++] = random_cons;
    } else if (rng_() % 1000 > (0 * 1000) / 7) {
        /* p=24% mutation */
        gp<List> random_list = List::from(gc_root_v_.at(rng_() % gc_root_v_.size()));
        if (!random_list.is_null()) {
            if (rng_() % 2 == 0) {
                /* pick up some random object, assign as head */
                gp<Object> random_car = gc_root_v_.at(rng_() % gc_root_v_.size());
                random_list->assign_head(random_car);
            } else {
                /* pick up some random object; if List, assign tail as tail */
                gp<List> random_cdr = List::from(gc_root_v_.at(rng_() % gc_root_v_.size()));
                if (!random_cdr.is_null() && !random_cdr->is_nil())
                    random_list->assign_rest(random_cdr->rest());
            }
        }
    }
    if (next_root_ >= gc_root_v_.size())
        this->next_root_ = 0;
}

void
AppState::generate_random_mutations() {
    for (int i = 0; i < this->alloc_per_cycle_; ++i) {
        this->generate_random_mutation();
    }
}

/* end AppState.cpp */
