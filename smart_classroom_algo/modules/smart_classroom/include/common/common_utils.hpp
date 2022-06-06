#ifndef SMART_CLASSROOM_CPP_COMMON_UTILS_HPP
#define SMART_CLASSROOM_CPP_COMMON_UTILS_HPP
namespace smc::common_utils {
    template<size_t...>
    struct tuple_idx {
    };

    template<size_t N, size_t... Is>
    struct tuple_bind : tuple_bind<N - 1, N - 1, Is...> {
    };

    template<size_t... Is>
    struct tuple_bind<0, Is...> {
        typedef tuple_idx<Is...> type;
    };
}
#endif //SMART_CLASSROOM_CPP_COMMON_UTILS_HPP
