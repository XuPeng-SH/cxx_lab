#pragma once

#include <cstdint>
#include <string>

using ErrorCode = int32_t;

constexpr ErrorCode PIPE_OK = 0;
constexpr ErrorCode PIPE_BASE = 10000;

constexpr ErrorCode
ToPipeErrorCode(const ErrorCode code) {
    return PIPE_BASE + code;
}

constexpr ErrorCode PIPELINE_OK = 0;
constexpr ErrorCode PIPELINE_BASE = 20000;

constexpr ErrorCode
ToPipelineErrorCode(const ErrorCode code) {
    return PIPELINE_BASE + code;
}

constexpr ErrorCode PIPE_EMPTY = ToPipeErrorCode(1);

constexpr ErrorCode PIPELINE_INITIALIZED = ToPipelineErrorCode(1);
constexpr ErrorCode PIPELINE_NOT_INITIALIZED = ToPipelineErrorCode(2);
constexpr ErrorCode PIPELINE_COMPLETED = ToPipelineErrorCode(3);
