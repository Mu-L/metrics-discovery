/*========================== begin_copyright_notice ============================

Copyright (C) 2023-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//     File Name:  md_metric_sets_SKL_GT2.h
//
//     Abstract:   C++ automated generated file to define specific for HW
//                 MetricSet classes inherited from CMetricSet

#pragma once
#include "md_calculation.h"
#include "md_concurrent_group.h"
#include "md_equation.h"
#include "md_information.h"
#include "md_metric.h"
#include "md_metric_set.h"

#if MD_INCLUDE_SKL_GT2_METRICS

namespace MetricsDiscoveryInternal::MetricSets_SKL_GT2_MMIO_Regs
{
    TCompletionCode AddInformationSet( CConcurrentGroup* concurrentGroup );

    class CMcRequestsMetricSet final : public CMetricSet
    {
    public:
        CMcRequestsMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CEdramTrafficMetricSet final : public CMetricSet
    {
    public:
        CEdramTrafficMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

} // namespace MetricsDiscoveryInternal::MetricSets_SKL_GT2_MMIO_Regs

namespace MetricsDiscoveryInternal::MetricSets_SKL_GT2_PipelineStatistics
{
    class CPipelineStatsMetricSet final : public CMetricSet
    {
    public:
        CPipelineStatsMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

} // namespace MetricsDiscoveryInternal::MetricSets_SKL_GT2_PipelineStatistics

namespace MetricsDiscoveryInternal::MetricSets_SKL_GT2_OA
{
    TCompletionCode AddInformationSet( CConcurrentGroup* concurrentGroup );

    class CRenderBasicMetricSet final : public CMetricSet
    {
    public:
        CRenderBasicMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CComputeBasicMetricSet final : public CMetricSet
    {
    public:
        CComputeBasicMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CRenderDX1xMetricSet final : public CMetricSet
    {
    public:
        CRenderDX1xMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CRenderPipeProfileMetricSet final : public CMetricSet
    {
    public:
        CRenderPipeProfileMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CMemoryReadsMetricSet final : public CMetricSet
    {
    public:
        CMemoryReadsMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CMemoryWritesMetricSet final : public CMetricSet
    {
    public:
        CMemoryWritesMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CComputeExtendedMetricSet final : public CMetricSet
    {
    public:
        CComputeExtendedMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CComputeL3CacheMetricSet final : public CMetricSet
    {
    public:
        CComputeL3CacheMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CHDCAndSFMetricSet final : public CMetricSet
    {
    public:
        CHDCAndSFMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CL3_1MetricSet final : public CMetricSet
    {
    public:
        CL3_1MetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CL3_2MetricSet final : public CMetricSet
    {
    public:
        CL3_2MetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CL3_3MetricSet final : public CMetricSet
    {
    public:
        CL3_3MetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CRasterizerAndPixelBackendMetricSet final : public CMetricSet
    {
    public:
        CRasterizerAndPixelBackendMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CSamplerMetricSet final : public CMetricSet
    {
    public:
        CSamplerMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CTDL_1MetricSet final : public CMetricSet
    {
    public:
        CTDL_1MetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CTDL_2MetricSet final : public CMetricSet
    {
    public:
        CTDL_2MetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CComputeExtraMetricSet final : public CMetricSet
    {
    public:
        CComputeExtraMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CVMEPipeMetricSet final : public CMetricSet
    {
    public:
        CVMEPipeMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CGpuBusynessMetricSet final : public CMetricSet
    {
    public:
        CGpuBusynessMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CFfBottlenecksMetricSet final : public CMetricSet
    {
    public:
        CFfBottlenecksMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CTestOaMetricSet final : public CMetricSet
    {
    public:
        CTestOaMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CPMA_StallMetricSet final : public CMetricSet
    {
    public:
        CPMA_StallMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

    class CAsyncComputeMetricSet final : public CMetricSet
    {
    public:
        CAsyncComputeMetricSet( CMetricsDevice& device, CConcurrentGroup* concurrentGroup, const char* symbolicName, const char* shortName, uint32_t apiMask, uint32_t category, uint32_t snapshotReportSize, uint32_t deltaReportSize, TReportType reportType, TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL, bool isCustom = false );

        TCompletionCode Initialize();
    };

} // namespace MetricsDiscoveryInternal::MetricSets_SKL_GT2_OA

#endif
