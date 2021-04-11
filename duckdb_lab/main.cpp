#include <iomanip>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <assert.h>
#include <atomic>
#include <memory>

#include <duckdb.hpp>
#include <duckdb/common/types/vector.hpp>
#include <thread>
#include <string>
#include <sstream>
#include <functional>
#include <gflags/gflags.h>
#include <ctime>

#include "driver.h"
#include "runner.h"
#include "Task.h"
#include "ThreadPool.h"
#include "TpccFactory.h"

using namespace std;

using namespace duckdb;

DEFINE_int32(threads, 1, "specify pragma threads. default 1");
DEFINE_int32(workers, 1, "specify worker threads");
DEFINE_uint32(sf, 1, "specify sf");
DEFINE_uint32(num, 100, "specify num task");
DEFINE_string(path, "/tmp/duckdb.tpcc", "db path");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    cout << "DBPATH: " << FLAGS_path << endl;
    cout << "WORKERS: " << FLAGS_workers << endl;
    cout << "THREADS: " << FLAGS_threads << endl;
    cout << "SF: " << FLAGS_sf << endl;

    /* for (auto i = 0; i < 100; ++i) { */
    /*     cout << RandomNumber<int>(1, 100) << endl; */
    /* } */
    TpccFactoryPtr factory;
    {
        auto sp = ScaleParameters::Build(FLAGS_sf);
        auto settings = TpccSettings::Build(sp);
        factory = TpccFactory::Build(settings);
    }
    auto db = std::make_shared<DuckDB>(FLAGS_path);

    // Below test can produce potential bug
    // If Do update before, the below select will take more than 2 seconds
    // FIXME
    {
        auto ss = chrono::high_resolution_clock::now();
        auto con = std::make_unique<Connection>(*db);
        auto ee = chrono::high_resolution_clock::now();
        con->BeginTransaction();
        /* con->Query("UPDATE CUSTOMER SET C_BALANCE = -3629.64, C_YTD_PAYMENT = 3629.64, C_PAYMENT_CNT = 2, C_DATA = '899 4 3 4 3 3619.64|wfjqabzipltzcrlmnunfjfnofzvqumcwyjmbwyxgqzcxjiwsuqdxmunpyqesodlalpckurwpuojuokpsizdzeknbzituuzbsdcybykregspjhecleztajjfydfttijbjxogrowiajlfuabpnenpkkxzhhlshmzapmzutasbmrkqfssbguprwgfetjfswfsbshgecrmfdwcskorrjudeozbrilnmejinwuqqzgetzbmuaozcmphedcvpavjmlxjebsniaqbkfsfuaaolchxjlrfmefmvhcswfuqijxegugvcjcgrmbmnvzsxwvogvntbfmwktyloazmndooieuxjhtabmfuyubbbjxemcmpkjvransyumssibfz' WHERE C_W_ID = 3 AND C_D_ID = 4 AND C_ID = 899"); */
        con->Commit();
        cout << std::this_thread::get_id() << " 11111 takes " << chrono::duration<double, std::milli>(chrono::high_resolution_clock::now()-ee).count() << endl;
        ee = chrono::high_resolution_clock::now();
        con->BeginTransaction();
        con->Query("SELECT C_ID, C_FIRST, C_MIDDLE, C_LAST, C_STREET_1, C_STREET_2, C_CITY, C_STATE, C_ZIP, C_PHONE, C_SINCE, C_CREDIT, C_CREDIT_LIM, C_DISCOUNT, C_BALANCE, C_YTD_PAYMENT, C_PAYMENT_CNT, C_DATA FROM CUSTOMER WHERE C_W_ID = 4 AND C_D_ID = 7 AND C_LAST = 'CALLYCALLYPRES' ORDER BY C_FIRST");
        cout << std::this_thread::get_id() << " 22222 takes " << chrono::duration<double, std::milli>(chrono::high_resolution_clock::now()-ee).count() << endl;
        ee = chrono::high_resolution_clock::now();
        con->Commit();

        ee = chrono::high_resolution_clock::now();
        cout << std::this_thread::get_id() << "Test takes " << chrono::duration<double, std::milli>(ee-ss).count() << endl;
    }

    int workers = FLAGS_workers;
    auto runner = make_shared<Runner>();
    for(auto i = 0; i < workers; ++i)
    {
        auto con = std::make_shared<Connection>(*db);
        if ((FLAGS_threads > 1) && (i == 0)) {
            con->Query(std::string("PRAGMA THREADS ") + std::to_string(FLAGS_threads));
        }

        if (i == 0) {
            con->Query("CREATE INDEX IDX_CUSTOMER ON CUSTOMER (C_W_ID,C_D_ID,C_LAST);");
        }
        auto driver = std::make_shared<Driver>(con);
        driver->db_ = db;
        runner->RegisterDriver(driver);
    }
    auto start = chrono::high_resolution_clock::now();
    {
        auto executor_pool = std::make_shared<ThreadPool>(workers);
        {
            for (auto i=0; i<FLAGS_num; ++i) {
                auto context = factory->NextContext();
                auto task = std::make_shared<Task>(context, runner);
                executor_pool->enqueue(std::bind(&Task::Run, task));
            }
        }
    }
    auto end = chrono::high_resolution_clock::now();
    cout << std::this_thread::get_id() << "Test takes " << chrono::duration<double, std::milli>(end-start).count() << endl;

    return 0;
}
