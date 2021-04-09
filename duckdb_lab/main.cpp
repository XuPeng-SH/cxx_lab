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

template<typename TYPE>
static void udf_vectorized(DataChunk &args, ExpressionState &state, Vector &result) {
	// set the result vector type
	/* result.vector_type = VectorType::FLAT_VECTOR; */
    result.SetVectorType(VectorType::FLAT_VECTOR);
	// get a raw array from the result
	auto result_data = FlatVector::GetData<TYPE>(result);

	// get the solely input vector
	auto &input = args.data[0];
	// now get an orrified vector
	VectorData vdata;
	input.Orrify(args.size(), vdata);

	// get a raw array from the orrified input
	auto input_data = (TYPE *)vdata.data;

	// handling the data
	for (idx_t i = 0; i < args.size(); i++) {
		auto idx = vdata.sel->get_index(i);
		/* if ((*vdata.nullmask)[idx]) { */
		/* 	continue; */
		/* } */
		result_data[i] = input_data[idx];
	}
}

struct Selection {
    explicit Selection(int count) {
        data_ = unique_ptr<int[]>(new int[count]);
    }

    unique_ptr<int[]> data_;
};

struct A {
    int size = 0;
    int a_[0];
};

DEFINE_int32(threads, 1, "specify pragma threads. default 1");
DEFINE_int32(workers, 1, "specify worker threads");
DEFINE_string(path, "/tmp/duckdb.tpcc", "db path");


int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    cout << "DBPATH: " << FLAGS_path << endl;
    cout << "WORKERS: " << FLAGS_workers << endl;
    cout << "THREADS: " << FLAGS_threads << endl;

    auto factory = TpccFactory::Build();
    {
        /* auto settings = TpccSettings::Build(); */
        /* auto mocker = TpccMocker(factory->GetSettings()); */
        auto mocker = factory->GetMocker();
        cout << "customer id " << mocker->MockCustomerID() << endl;
        cout << "item id " << mocker->MockCustomerID() << endl;
        cout << "wh id " << mocker->MockWarehouseID() << endl;
        cout << "district id " << mocker->MockDistrictID() << endl;
    }

    auto db = std::make_shared<DuckDB>(FLAGS_path);

    int workers = FLAGS_workers;
    auto runner = make_shared<Runner>();
    for(auto i = 0; i < workers; ++i)
    {
        auto con = std::make_shared<Connection>(*db);
        if (FLAGS_threads > 1) {
            con->Query(std::string("PRAGMA THREADS ") + std::to_string(FLAGS_threads));
        }
        auto driver = std::make_shared<Driver>(con);
        driver->db_ = db;
        runner->RegisterDriver(driver);
    }
    auto start = chrono::high_resolution_clock::now();
    {
        auto executor_pool = std::make_shared<ThreadPool>(workers);
        {
            /* auto context = std::make_shared<TpccContext>(); */
            /* context->delivery_ctx_->ol_delivery_d = "1992-01-01 12:00:00"; */
            for (auto i=0; i<100; ++i) {
                auto context = factory->NextContext();
                auto task = std::make_shared<DeliveryTask>(context, runner);
                executor_pool->enqueue(std::bind(&DeliveryTask::Run, task));
            }
        }
    }
    auto end = chrono::high_resolution_clock::now();
    cout << std::this_thread::get_id() << "Test takes " << chrono::duration<double, std::milli>(end-start).count() << endl;



    return 0;
    /* Connection con(db); */
    /* /1* con.Query("INSERT INTO integers VALUES (5000000), (50000001)"); *1/ */
    /* /1* auto result = con.Query("SELECT i FROM integers WHERE i == 10000;"); *1/ */
    /* /1* auto result = con.Query("select CAST('12.5' AS REAL) as Real"); *1/ */
    /* /1* auto result = con.Query("SELECT 6 & 3"); *1/ */
    /* /1* auto result = con.Query("select 3*3 == 9 AND 9 = 9;"); *1/ */
    /* /1* auto result = con.Query("select grade from ss where cid=202332"); *1/ */
    /* /1* auto result = con.Query("SELECT name FROM students JOIN exams USING (sid) WHERE name LIKE 'Ma%'"); *1/ */
    /* /1* auto result = con.Query("BEGIN TRANSACTION"); *1/ */
    /* /1* result = con.Query("INSERT INTO ss (cid) VALUES (99999)"); *1/ */
    /* /1* result = con.Query("END TRANSACTION"); *1/ */
    /* /1* result = con.Query("select grade from ss where cid=202332"); *1/ */
    /* /1* result = con.Query("INSERT INTO ss (cid) VALUES (1000022020)"); *1/ */

    /* auto result = con.Query("DROP TABLE ss"); */
    /* cout << result->ToString() << endl; */
    /* result = con.Query("CREATE TABLE ss(cid INTEGER, grade INTEGER, PRIMARY KEY(cid));"); */
    /* cout << result->ToString() << endl; */
    /* /1* auto result = con.Query("delete from ss where cid = 2"); *1/ */
    /* /1* cout << result->ToString() << endl; *1/ */
    /* /1* result = con.Query("delete from ss where cid = 3"); *1/ */
    /* /1* cout << result->ToString() << endl; *1/ */
    /* result = con.Query("INSERT INTO ss VALUES (2, 2222)"); */
    /* cout << result->ToString() << endl; */
    /* result = con.Query("UPDATE ss SET cid = 3 where cid = 2"); */
    /* cout << result->ToString() << endl; */
    /* result = con.Query("update ss set grade = 12332 where cid = 3"); */
    /* cout << result->ToString() << endl; */
    /* result = con.Query("select * from ss where cid=3"); */
    /* cout << result->ToString() << endl; */

    /* return 0; */
    /* /1* con.Query("CREATE TABLE integers (i INTEGER)"); *1/ */
    /* int first, second, third; */
    /* int step = 50; */
    /* for (int i = 900000; i < 920000; i+=step) { */
    /*     stringstream ss; */
    /*     ss << "BEGIN TRANSACTION;INSERT INTO ss VALUES "; */
    /*     bool first = true; */
    /*     for (int j = 0; j < step; ++j) { */
    /*         if (first) { */
    /*             first = false; */
    /*         } else { */
    /*             ss << ","; */
    /*         } */
    /*         ss << "(" << (i + j) << "," << (i+j) << ")"; */
    /*     } */
    /*     ss << "; END TRANSACTION;"; */
    /*     /1* cout << ss.str() << endl; *1/ */
    /*     con.Query(ss.str()); */
    /* } */
    /* /1* con.Query("INSERT INTO integers VALUES (1), (2), (3), (999)"); *1/ */

    /* /1* con.CreateVectorizedFunction<int, int>("udf_vectorized_int", &udf_vectorized<int>); *1/ */

    /* /1* con.Query("SELECT udf_vectorized_int(i) FROM integers")->Print(); *1/ */

    /* /1* auto f = [&](int i){ *1/ */
    /* /1*         Connection con(db); *1/ */
    /* /1*         auto table = string("integers"); *1/ */
    /* /1*         table += to_string(i); *1/ */
    /* /1*         auto st1 = string("CREATE TABLE ") + table + "(i INTEGER)"; *1/ */
    /* /1*         con.Query(st1); *1/ */
    /* /1*         auto st2 = string("INSERT INTO ") + table + " VALUES (3)"; *1/ */
    /* /1*         con.Query(st2); *1/ */
    /* /1*         auto st3 = string("SELECT * FROM ") + table; *1/ */
    /* /1*         auto result = con.Query(st3); *1/ */
    /* /1*         result->Print(); *1/ */

    /* /1* }; *1/ */

    /* /1* vector<thread> ids; *1/ */
    /* /1* for (int i = 0; i < 4; ++i) { *1/ */
    /* /1*     auto t = thread(f, i); *1/ */
    /* /1*     ids.push_back(move(t)); *1/ */
    /* /1* } *1/ */

    /* /1* for (auto& t : ids) { *1/ */
    /* /1*     t.join(); *1/ */
    /* /1* } *1/ */
    /* return 0; */
}
