#include <iostream>
#include <thread>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <assert.h>
#include <atomic>
#include <folly/executors/ThreadedExecutor.h>
#include <folly/futures/Future.h>


using namespace std;

template<class T>
void func(T&& param) {
    if (std::is_same<T,int>::value)
        std::cout << "param is an int\n";
    else
        std::cout << "param is not an int\n";
    /* if (std::is_same<typename std::decay<T>::type,int>::value) */
    /*     std::cout << "param is an int\n"; */
    /* else */
    /*     std::cout << "param is not an int\n"; */
}

void lab1() {
    int three = 3;
    func(three);  //prints "param is not an int"!!!!
}

int main(int argc, char** argv) {
    auto do_print = [](int v) {
        /* throw runtime_error("xx"); */
        cout << "do_print " << v << endl;
    };

    auto error = [](auto&& e) {
        if (e)
        cout << "catch error! " << e.what().c_str() << endl;
    };

    folly::ThreadedExecutor executor;
    folly::Promise<int> promise;
    auto future = promise.getSemiFuture().via(&executor).thenValue(do_print).thenError(error);
    cout << "fullfilled? " << promise.isFulfilled() << endl;
    cout << "Fullfilling promise" << endl;
    promise.setValue(1);
    cout << "fullfilled? " << promise.isFulfilled() << endl;
    cout << "done " << endl;
    future.wait();
    cout << future.hasException() << endl;
    cout << future.hasValue() << endl;
    cout << future.isReady() << endl;

    auto s = folly::to<std::string>(20.4);
    cout << s << endl;

    folly::via(&executor, [] () {
        cout << "dummy run" << endl;
    });

    lab1();

    return 0;
}
