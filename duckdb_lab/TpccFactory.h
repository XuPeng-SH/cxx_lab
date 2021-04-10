#pragma once
#include <memory>
#include <sstream>
#include <stdint.h>
#include <stddef.h>
#include <set>
#include <vector>
#include <algorithm>
#include "Utils.h"
#include "types.h"
#include "Context.h"

enum class TransactionType : uint8_t {
    INVALID = 0,
    STOCK_LEVEL = 1,
    DELIVERY = 2,
    ORDER_STATUS = 3,
    PAYMENT = 4,
    NEW_ORDER = 5,
};

struct ScaleParameters;
using ScaleParametersPtr = std::shared_ptr<ScaleParameters>;
struct ScaleParameters {
    size_t warehouses_;
    size_t items_;
    size_t districts_per_wh_;
    size_t customers_per_district_;
    size_t new_orders_per_district_;
    ID_TYPE wh_start_;
    ID_TYPE wh_end_;

    static ScaleParametersPtr
    Build(size_t scale_factor = 1);
};

struct TpccSettings;
using TpccSettingsPtr = std::shared_ptr<TpccSettings>;
struct TpccSettings {
    bool
    IsValid() const;

    explicit TpccSettings(ScaleParametersPtr sp);

    TpccSettings() = delete;

    /* int stock_level_p_ = 4; */
    /* int delivery_p_ = 4; */
    /* int order_status_p_ = 4; */
    /* int payment_p_ = 43; */
    /* int new_order_p_ = 45; */
    int stock_level_p_ = 1;
    int delivery_p_ = 4;
    int order_status_p_ = 50;
    int payment_p_ = 1;
    int new_order_p_ = 44;

    mutable int sl_p_upper_ = 0;
    mutable int d_p_upper_ = 0;
    mutable int os_p_upper_ = 0;
    mutable int p_p_upper_ = 0;

    ScaleParametersPtr sp_;

    int GetStockLevelUpper() const { return sl_p_upper_; }
    int GetDeliveryUpper() const { return d_p_upper_; }
    int GetOrderStatusUpper() const { return os_p_upper_; }
    int GetPaymentUpper() const { return p_p_upper_; }

    static TpccSettingsPtr
    Build(ScaleParametersPtr sp = nullptr) {
        if (!sp) {
            sp = ScaleParameters::Build();
        }
        return std::make_shared<TpccSettings>(sp);
    }
};

struct TpccMocker;
using TpccMockerPtr = std::shared_ptr<TpccMocker>;
struct TpccMocker {
    static const std::vector<std::string> SYLLABLES;

    TpccSettingsPtr settings_;

    explicit TpccMocker(TpccSettingsPtr settings) : settings_(settings) {}

    TpccMocker() = delete;

    static TpccMockerPtr
    Build(TpccSettingsPtr settings = nullptr) {
        if (!settings) {
            settings = TpccSettings::Build();
        }
        return std::make_shared<TpccMocker>(settings);
    }

    ID_TYPE
    MockWarehouseID() {
        return RandomNumber<ID_TYPE>(settings_->sp_->wh_start_, settings_->sp_->wh_end_);
    }

    ID_TYPE
    MockDistrictID() {
        return RandomNumber<ID_TYPE>(1, settings_->sp_->districts_per_wh_);
    }
    std::string
    MockLastName() {
        auto end = std::min<int>(settings_->sp_->customers_per_district_ - 1, 999);
        auto number = RNGenerator::GetInstance().Produce(RNGenerator::IDType::LAST_NAME, 0, end);
        std::vector<int> indicies = { int(number/100), (int(number/10))%10, number%10 };
        std::stringstream ss;
        for (auto idx : indicies) {
            ss << SYLLABLES[idx];
        }
        return ss.str();
    }
    ID_TYPE
    MockCustomerID() {
        return RNGenerator::GetInstance().Produce(RNGenerator::IDType::CUSTOMER, 1,
                settings_->sp_->customers_per_district_);
    }
    ID_TYPE
    MockItemID() {
        return RNGenerator::GetInstance().Produce(RNGenerator::IDType::ITEM, 1,
                settings_->sp_->items_);
    }
    IDS_TYPE
    MockItemIDS(size_t n) {
        IDS_TYPE ids;
        std::set<ID_TYPE> id_set;
        size_t size = 0;
        while (ids.size() >= n) {
            auto id = MockItemID();
            size = id_set.size();
            id_set.insert(id);
            if (size == id_set.size()) {
                continue;
            }
            ids.push_back(id);
        }
        return std::move(ids);
    }
};

class TpccFactory;
using TpccFactoryPtr = std::shared_ptr<TpccFactory>;
class TpccFactory {
 public:
     TpccSettingsPtr
     GetSettings() {
         return settings_;
     }

     TpccMockerPtr
     GetMocker() {
         return mocker_;
     }

     TpccContextPtr
     NextContext();

     static TpccFactoryPtr
     Build(TpccSettingsPtr settings = nullptr) {
         if (!settings) {
             settings = TpccSettings::Build();
         }
         if (!settings->IsValid()) {
             return nullptr;
         }
         return std::make_shared<TpccFactory>(settings);
     }

     TpccFactory(TpccSettingsPtr settings) : settings_(settings) {
         mocker_ = TpccMocker::Build(settings_);
     }

 protected:
     TpccFactory() = delete;

     TpccSettingsPtr settings_;
     TpccMockerPtr mocker_;
};
