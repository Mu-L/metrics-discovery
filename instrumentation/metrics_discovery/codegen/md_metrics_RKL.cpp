﻿/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//     File Name:  md_metrics_RKL.cpp

//     Abstract:   C++ automated generated file to stack metrics meta data

#include "md_per_platform_preamble.h"
#include "md_metric_sets_RKL.h"

using namespace MetricsDiscoveryInternal;

#if( ( !defined( MD_INCLUDE_RKL_METRICS ) && MD_INCLUDE_ALL_METRICS ) || MD_INCLUDE_RKL_METRICS )

TCompletionCode CreateMetricTreeRKL_PipelineStatistics( CMetricsDevice* metricsDevice, CConcurrentGroup* concurrentGroup )
{
    const uint32_t adapterId = OBTAIN_ADAPTER_ID( metricsDevice );

    MD_LOG_ENTER_A( adapterId );
    MD_CHECK_PTR_RET_A( adapterId, metricsDevice, CC_ERROR_INVALID_PARAMETER );
    MD_CHECK_PTR_RET_A( adapterId, concurrentGroup, CC_ERROR_INVALID_PARAMETER );

    CMetricSet*      metricSet                                               = nullptr;
    uint8_t          platformMaskByteArray[MD_PLATFORM_MASK_BYTE_ARRAY_SIZE] = {};
    TByteArrayLatest platformMask                                            = { MD_PLATFORM_MASK_BYTE_ARRAY_SIZE, platformMaskByteArray };

    MD_CHECK_CC( SetPlatformMask( adapterId, &platformMask, nullptr, false, GENERATION_RKL ) );

    if( metricsDevice->IsPlatformTypeOf( &platformMask ) )
    {
        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_PipelineStatistics::CPipelineStatsMetricSet>( "PipelineStats", "Pipeline Statistics for OGL4", API_TYPE_OGL | API_TYPE_OGL4_X,
            GPU_RENDER | GPU_COMPUTE, 0, 96, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );
    }

    MD_LOG_EXIT_A( adapterId );
    return CC_OK;

exception:
    MD_LOG_EXIT_A( adapterId );
    return CC_ERROR_NO_MEMORY;
}
#endif

#if( ( !defined( MD_INCLUDE_RKL_METRICS ) && MD_INCLUDE_ALL_METRICS ) || MD_INCLUDE_RKL_METRICS )

TCompletionCode CreateMetricTreeRKL_OA( CMetricsDevice* metricsDevice, CConcurrentGroup* concurrentGroup )
{
    const uint32_t adapterId = OBTAIN_ADAPTER_ID( metricsDevice );

    MD_LOG_ENTER_A( adapterId );
    MD_CHECK_PTR_RET_A( adapterId, metricsDevice, CC_ERROR_INVALID_PARAMETER );
    MD_CHECK_PTR_RET_A( adapterId, concurrentGroup, CC_ERROR_INVALID_PARAMETER );

    CMetricSet*      metricSet                                               = nullptr;
    uint8_t          platformMaskByteArray[MD_PLATFORM_MASK_BYTE_ARRAY_SIZE] = {};
    TByteArrayLatest platformMask                                            = { MD_PLATFORM_MASK_BYTE_ARRAY_SIZE, platformMaskByteArray };

    MD_CHECK_CC( SetPlatformMask( adapterId, &platformMask, nullptr, false, GENERATION_RKL ) );

    if( metricsDevice->IsPlatformTypeOf( &platformMask ) )
    {
        MD_CHECK_CC( MetricSets_RKL_OA::AddInformationSet( concurrentGroup ) );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CRenderBasicMetricSet>( "RenderBasic", "Render Metrics Basic set", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CComputeBasicMetricSet>( "ComputeBasic", "Compute Metrics Basic set", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CRenderPipeProfileMetricSet>( "RenderPipeProfile", "Render Metrics set for 3D Pipeline Profile", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CHDCAndSFMetricSet>( "HDCAndSF", "Metric set HDCAndSF", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CRasterizerAndPixelBackendMetricSet>( "RasterizerAndPixelBackend", "Metric set RasterizerAndPixelBackend", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CL3_1MetricSet>( "L3_1", "L3_1", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CL3_2MetricSet>( "L3_2", "L3_2", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CL3_3MetricSet>( "L3_3", "L3_3", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CL3_4MetricSet>( "L3_4", "L3_4", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CL3_5MetricSet>( "L3_5", "L3_5", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CL3_6MetricSet>( "L3_6", "L3_6", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CSampler_1MetricSet>( "Sampler_1", "Sampler_1", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CTDL_1MetricSet>( "TDL_1", "TDL_1", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CGpuBusynessMetricSet>( "GpuBusyness", "GpuBusyness", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CEuActivity1MetricSet>( "EuActivity1", "EuActivity1", API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_VULKAN | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CEuActivity2MetricSet>( "EuActivity2", "EuActivity2", API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_VULKAN | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CEuActivity3MetricSet>( "EuActivity3", "EuActivity3", API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_VULKAN | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CEuActivity4MetricSet>( "EuActivity4", "EuActivity4", API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_VULKAN | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CEuActivity5MetricSet>( "EuActivity5", "EuActivity5", API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_VULKAN | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CEuActivity6MetricSet>( "EuActivity6", "EuActivity6", API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_VULKAN | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CEuActivity7MetricSet>( "EuActivity7", "EuActivity7", API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_VULKAN | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CEuActivity8MetricSet>( "EuActivity8", "EuActivity8", API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_VULKAN | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CTestOaMetricSet>( "TestOa", "Metric set TestOa", API_TYPE_VULKAN | API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );

        metricSet = concurrentGroup->AddMetricSetExplicit<MetricSets_RKL_OA::CAsyncComputeMetricSet>( "AsyncCompute", "AsyncCompute", API_TYPE_OGL | API_TYPE_OGL4_X | API_TYPE_VULKAN | API_TYPE_OCL | API_TYPE_IOSTREAM,
            GPU_RENDER | GPU_COMPUTE | GPU_MEDIA | GPU_GENERIC, 256, 672, OA_REPORT_TYPE_256B_A45_NOA16, &platformMask, nullptr );
        MD_CHECK_PTR( metricSet );
    }

    MD_LOG_EXIT_A( adapterId );
    return CC_OK;

exception:
    MD_LOG_EXIT_A( adapterId );
    return CC_ERROR_NO_MEMORY;
}
#endif
