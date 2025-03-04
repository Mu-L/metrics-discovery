/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//     File Name:  md_metric.h

//     Abstract:   C++ Metrics Discovery internal metric header

#pragma once

#include "md_types.h"

#include <cstdio>

using namespace MetricsDiscovery;

namespace MetricsDiscoveryInternal
{
    ///////////////////////////////////////////////////////////////////////////////
    // Forward declarations:                                                     //
    ///////////////////////////////////////////////////////////////////////////////
    class CMetricsDevice;

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CMetric
    //
    // Description:
    //     The metric that is sampled. Stores all metric information.
    //
    //////////////////////////////////////////////////////////////////////////////
    class CMetric : public IMetricLatest
    {
    public:
        // API 1.13:
        virtual TMetricParamsLatest* GetParams( void );

    public:
        // Constructor & Destructor:
        CMetric(
            CMetricsDevice&   device,
            uint32_t          id,
            const char*       name,
            const char*       shortName,
            const char*       longName,
            const char*       group,
            uint32_t          groupId,
            uint32_t          usageFlagsMask,
            uint32_t          apiMask,
            TMetricType       metricType,
            TMetricResultType resultType,
            const char*       units,
            int64_t           loWatermark,
            int64_t           hiWatermark,
            THwUnitType       hwType,
            const char*       alias,
            const char*       signalName,
            bool              isCustom = false );
        explicit CMetric( const CMetric& other );
        virtual ~CMetric();

        CMetric& operator=( const CMetric& ) = delete; // Delete assignment operator

        // Non-API:
        TCompletionCode SetSnapshotReportReadEquation( const char* equationString );
        TCompletionCode SetDeltaReportReadEquation( const char* equationString );
        TCompletionCode SetNormalizationEquation( const char* equationString );
        TCompletionCode SetSnapshotReportDeltaFunction( const char* equationString );
        TCompletionCode SetSnapshotReportDeltaFunction( TDeltaFunction_1_0 deltaFunction );
        TCompletionCode SetAvailabilityEquation( const char* equationString );
        TCompletionCode SetMaxValueEquation( const char* equationString );
        void            SetIdInSetParam( uint32_t id );
        void            SetQueryModeMask( const uint32_t queryModeMask );

        uint32_t    GetId() const;
        const char* GetSignalName();
        bool        IsAvailabilityEquationTrue();

        TCompletionCode WriteCMetricToBuffer( uint8_t* buffer, uint32_t& bufferSize, uint32_t& bufferOffset );

    private:
        // Variables:
        TMetricParamsLatest m_params;
        uint32_t            m_id;       // Position in set before any filterings (SetApiFiltering, AvailableEquation check)
        bool                m_isCustom; // true if metric was created from AddCustomMetric function

        const char*     m_signalName;
        CEquation*      m_availabilityEquation;
        CEquation*      m_ioReadEquation;
        CEquation*      m_queryReadEquation;
        CEquation*      m_normEquation;
        CEquation*      m_maxValueEquation;
        CMetricsDevice& m_device;
    };
} // namespace MetricsDiscoveryInternal
