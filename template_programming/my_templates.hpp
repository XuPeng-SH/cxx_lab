
template <class Arg1T, class RestT>
struct unary_function {
    typedef Arg1T first_argument_type;
    typedef RestT result_type;
};

template<typename T, int v>
struct ValueHelper {
    enum { value = v };
};

struct FunctionHelper {
    template<typename T>
        static ValueHelper<typename T::first_argument_type, 1> has_first_argument(typename T::first_argument_type);
    template<typename T>
        static ValueHelper<T, 0> has_first_argument(...);
    template<typename T>
        static ValueHelper<typename T::second_argument_type, 1> has_second_argument(typename T::second_argument_type);
    template<typename T>
        static ValueHelper<T, 0> has_second_argument(...);
};

template<typename T>
struct FunctionChecker {
    typedef decltype(FunctionHelper::has_first_argument<T>(0)) FirstArgumentWrapType;
    enum { has_first_argument = FirstArgumentWrapType::value == 1};

    typedef decltype(FunctionHelper::has_second_argument<T>(0)) SecondArgumentWrapType;
    enum { has_second_argument = SecondArgumentWrapType::value == 1};
};


#define HAS_CLASS_TYPE(CLASS, TYPE) \
    has_##TYPE<CLASS>::value

#define DEFINE_HAS_CLASS_TYPE(TYPE) \
template<typename C> \
struct has_##TYPE { \
    struct _yes {char c;}; \
    struct _no {_yes a[2];}; \
    template <typename T> \
    static _no dummy(...); \
                            \
    template <typename T> \
    static _yes dummy(typename T::TYPE); \
    static const bool value = sizeof(dummy<C>(0)) == sizeof(_yes); \
};

/* template<typename C> */
/* struct has_second_argument { */
/*     struct _yes {char c;}; */
/*     struct _no {_yes a[2];}; */

/*     template<typename T> */
/*         static _no dummy(...); */

/*     template<typename T> */
/*         static _yes dummy(typename T::second_argument_type); */

/*     static const bool value = sizeof(dummy<C>(0)) == sizeof(_yes); */
/* }; */

template<typename Operation, typename T>
class MyBinder2nd : public unary_function<typename Operation::first_argument_type,
                                          typename Operation::result_type>  {

protected:
    Operation mOp;
    typename Operation::second_argument_type mValue;
public:
    MyBinder2nd(const Operation& op_, const typename Operation::second_argument_type& arg_) : mOp(op_), mValue(arg_) {}

    typename Operation::result_type operator() (const typename Operation::first_argument_type& arg1) const {
        return mOp(arg1, mValue);
    }
};
