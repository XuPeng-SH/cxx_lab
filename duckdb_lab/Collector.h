#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <string>

struct MetricEntry {
    size_t exec_time_us_ = 0;
    bool is_commit_ = true;
    uint64_t ts_ = 0;
};

struct Metric {
    void
    Record(MetricEntry&& entry) {
        std::lock_guard<std::mutex> lk(mtx_);
        entries_.push_back(std::move(entry));
        if (entry.ts_ > last_modify_ts_) {
            last_modify_ts_ = entry.ts_;
        }
    }
    std::vector<MetricEntry> entries_;
    std::mutex mtx_;

    void
    Summary() {
        std::lock_guard<std::mutex> lk(mtx_);
        if (last_summary_ts_ >= last_modify_ts_) {
            return;
        }
        Reset();
        totals_ = entries_.size();
        float total_la = 0;
        for (auto& entry : entries_) {
            if (entry.is_commit_) {
                commits_++;
                total_commits_time_ += entry.exec_time_us_;
            } else {
                rollbacks_++;
                total_rollback_time_ += entry.exec_time_us_;
            }
            total_la += entry.exec_time_us_;
        }
        assert(commits_ + rollbacks_ == totals_);
        if (totals_ != 0) {
            rate_ = rollbacks_ / (float)totals_;
            rate_str_ = std::to_string(rate_*100) + "%";
            avg_la_ = (float)(total_la) / totals_;
        }
        last_summary_ts_ = last_modify_ts_;
    }

    void
    Reset() {
        commits_ = 0;
        rollbacks_ = 0;
        totals_ = 0;
        total_commits_time_ = 0;
        total_rollback_time_ = 0;
        avg_la_ = 0.0;
        rate_ = 0.0;
        rate_str_ = "";
    }
    size_t commits_ = 0;
    size_t rollbacks_ = 0;
    size_t totals_ = 0;
    float rate_ = 0.0;
    std::string rate_str_ = "";
    uint64_t total_commits_time_ = 0;
    uint64_t total_rollback_time_ = 0;
    uint64_t last_summary_ts_ = 0;
    uint64_t last_modify_ts_ = 0;
    float avg_la_ = 0.0;
};

using MetricPtr = std::shared_ptr<Metric>;

class Collector {
 public:
    Collector() {
        d_m_ = std::make_shared<Metric>();
        no_m_ = std::make_shared<Metric>();
        os_m_ = std::make_shared<Metric>();
        sl_m_ = std::make_shared<Metric>();
        p_m_ = std::make_shared<Metric>();
    }
    void
    RecordDeliveryEntry(MetricEntry&& entry) {
        d_m_->Record(std::move(entry));
    }
    void
    RecordOrderEntry(MetricEntry&& entry) {
        os_m_->Record(std::move(entry));
    }
    void
    RecordNewOrderEntry(MetricEntry&& entry) {
        no_m_->Record(std::move(entry));
    }
    void
    RecordStockLevelEntry(MetricEntry&& entry) {
        sl_m_->Record(std::move(entry));
    }
    void
    RecordPaymentEntry(MetricEntry&& entry) {
        p_m_->Record(std::move(entry));
    }

    template <typename T> void
    FormatCol(T num, std::ostream& ss, int w = 11) const {
        ss << std::left << std::setw(w) << num;
    }

    std::string
    ToString() const {
        sl_m_->Summary();
        d_m_->Summary();
        os_m_->Summary();
        p_m_->Summary();
        no_m_->Summary();

        std::stringstream ss;
        ss << "==============================================================\n";
        ss << "Execution Result after " << ((float)total_time_ / 1000) << "(ms)\n";
        ss << "--------------------------------------------------------------\n";
        FormatCol<std::string>("OP", ss, 14);
        FormatCol<std::string>("Executed", ss);
        FormatCol<std::string>("Committed", ss);
        FormatCol<std::string>("Rollback", ss);
        FormatCol<std::string>("Rate", ss);
        FormatCol<std::string>("AvgLa(ms)", ss);
        ss << "\n";
        FormatCol<std::string>("STOCK_LEVEL", ss, 14);
        FormatCol<uint64_t>(sl_m_->totals_, ss);
        FormatCol<uint64_t>(sl_m_->commits_, ss);
        FormatCol<uint64_t>(sl_m_->rollbacks_, ss);
        FormatCol<std::string>(sl_m_->rate_str_, ss);
        FormatCol<float>(sl_m_->avg_la_ / 1000, ss);
        ss << "\n";
        FormatCol<std::string>("DELIVERY", ss, 14);
        FormatCol<uint64_t>(d_m_->totals_, ss);
        FormatCol<uint64_t>(d_m_->commits_, ss);
        FormatCol<uint64_t>(d_m_->rollbacks_, ss);
        FormatCol<std::string>(d_m_->rate_str_, ss);
        FormatCol<float>(d_m_->avg_la_ / 1000, ss);
        ss << "\n";
        FormatCol<std::string>("ORDER_STATUS", ss, 14);
        FormatCol<uint64_t>(os_m_->totals_, ss);
        FormatCol<uint64_t>(os_m_->commits_, ss);
        FormatCol<uint64_t>(os_m_->rollbacks_, ss);
        FormatCol<std::string>(os_m_->rate_str_, ss);
        FormatCol<float>(os_m_->avg_la_ / 1000, ss);
        ss << "\n";
        FormatCol<std::string>("PAYMENT", ss, 14);
        FormatCol<uint64_t>(p_m_->totals_, ss);
        FormatCol<uint64_t>(p_m_->commits_, ss);
        FormatCol<uint64_t>(p_m_->rollbacks_, ss);
        FormatCol<std::string>(p_m_->rate_str_, ss);
        FormatCol<float>(p_m_->avg_la_ / 1000, ss);
        ss << "\n";
        FormatCol<std::string>("NEW_ORDER", ss, 14);
        FormatCol<uint64_t>(no_m_->totals_, ss);
        FormatCol<uint64_t>(no_m_->commits_, ss);
        FormatCol<uint64_t>(no_m_->rollbacks_, ss);
        FormatCol<std::string>(no_m_->rate_str_, ss);
        FormatCol<float>(no_m_->avg_la_ / 1000, ss);
        ss << "\n";
        ss << "--------------------------------------------------------------\n";
        FormatCol<std::string>("TOTAL", ss, 14);
        FormatCol<uint64_t>(sl_m_->totals_ + d_m_->totals_ + os_m_->totals_ + p_m_->totals_ + no_m_->totals_, ss);
        return ss.str();
    }

    void
    RecordTotalTime(uint64_t us) {
        total_time_ = us;
    }

 protected:
    MetricPtr d_m_;
    MetricPtr no_m_;
    MetricPtr os_m_;
    MetricPtr sl_m_;
    MetricPtr p_m_;

    uint64_t total_time_ = 0;
};

using CollectorPtr = std::shared_ptr<Collector>;
