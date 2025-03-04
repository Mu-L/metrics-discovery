/*========================== begin_copyright_notice ============================

Copyright (C) 2023-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//     File Name:  md_metrics_calculator.h

//     Abstract:   C++ metrics discovery metrics calculation features header with
//                 inline functions implementation.

#pragma once

#include "md_adapter.h"
#include "md_equation.h"
#include "md_information.h"
#include "md_metric.h"
#include "md_metrics_device.h"
#include "md_metric_set.h"
#include "md_types.h"

#include <cstring>
#include <limits>
#include <stack>

namespace MetricsDiscoveryInternal
{
    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CMetricsCalculator
    //
    // Description:
    //     Class wrapping operations like raw values read or normalization on a single report.
    //
    //////////////////////////////////////////////////////////////////////////////
    class CMetricsCalculator
    {
    public:
        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CMetricsCalculator
        //
        // Description:
        //     CMetricsCalculator constructor.
        //
        // Input:
        //     CMetricsDevice& metricsDevice - metrics device is used for obtaining global symbols
        //                                     during calculations
        //
        //////////////////////////////////////////////////////////////////////////////
        inline CMetricsCalculator( CMetricsDevice& metricsDevice )
            : m_readEquationStack{}
            , m_readEquationAndDeltaStack{}
            , m_normalizationEquationStack{}
            , m_device( metricsDevice )
            , m_gpuCoreClocks( 0 )
            , m_euCoresCount( 0 )
            , m_savedReport( nullptr )
            , m_savedReportSize( 0 )
            , m_contextIdPrev( 0 )
            , m_savedReportPresent( false )
            , m_prevValues( nullptr )
            , m_prevValuesCount( 0 )
        {
            TTypedValue_1_0* euCoresTotalCount = GetGlobalSymbolValue( "VectorEngineTotalCount" );
            // Get old global symbol if new one is not available
            if( euCoresTotalCount == nullptr )
            {
                euCoresTotalCount = GetGlobalSymbolValue( "EuCoresTotalCount" );
            }
            m_euCoresCount = euCoresTotalCount ? euCoresTotalCount->ValueUInt32 : 0;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     ~CMetricsCalculator
        //
        // Description:
        //     CMetricsCalculator destructor.
        //
        //////////////////////////////////////////////////////////////////////////////
        inline ~CMetricsCalculator()
        {
            MD_SAFE_DELETE_ARRAY( m_savedReport );
            MD_SAFE_DELETE_ARRAY( m_prevValues );
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CMetricsCalculator
        //
        // Description:
        //     CMetricsCalculator Delete copy-constructor.
        //
        //////////////////////////////////////////////////////////////////////////////
        inline CMetricsCalculator( const CMetricsCalculator& ) = delete; // Delete copy-constructor

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     operator=
        //
        // Description:
        //     Delete assignment operator.
        //
        //////////////////////////////////////////////////////////////////////////////
        inline CMetricsCalculator& operator=( const CMetricsCalculator& ) = delete; // Delete assignment operator

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //    CMetricsCalculator
        //
        // Method:
        //     Reset
        //
        // Description:
        //     Reset Calculator to the initial state.
        //     Allocate memory to store last raw and previous calculated report for future calculation.
        //
        // Input:
        //     uint32_t rawReportSize              - Raw report size to allocate memory for report to save.
        //     uint32_t metricsAndInformationCount - Metrics and information count to allocate memory for previous calculated report.
        //
        //////////////////////////////////////////////////////////////////////////////
        inline void Reset( uint32_t rawReportSize = 0, uint32_t metricsAndInformationCount = 0 )
        {
            m_gpuCoreClocks = 0;

            if( m_savedReportSize != rawReportSize && rawReportSize > 0 )
            {
                MD_SAFE_DELETE_ARRAY( m_savedReport );
                m_savedReport = new( std::nothrow ) uint8_t[rawReportSize];
                if( m_savedReport == nullptr )
                {
                    MD_LOG_A( m_device.GetAdapter().GetAdapterId(), LOG_ERROR, "error allocating saved report memory" );
                    m_savedReportSize = 0;
                }
                else
                {
                    m_savedReportSize = rawReportSize;
                }
                m_savedReportPresent = false;
            }

            if( m_prevValuesCount != metricsAndInformationCount && metricsAndInformationCount > 0 )
            {
                MD_SAFE_DELETE_ARRAY( m_prevValues );
                m_prevValues = new( std::nothrow ) TTypedValue_1_0[metricsAndInformationCount]();
                if( m_prevValues == nullptr )
                {
                    MD_LOG_A( m_device.GetAdapter().GetAdapterId(), LOG_ERROR, "error allocating prev values memory" );
                    m_prevValuesCount = 0;
                }
                else
                {
                    m_prevValuesCount = metricsAndInformationCount;
                }
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     ReadMetricsFromQueryReport
        //
        // Description:
        //     Reads metrics from a given metric set using raw report data.
        //
        // Input:
        //     const uint8_t*   rawReport - (IN) single raw report
        //     TTypedValue_1_0* outValues - (OUT) single output report
        //     CMetricSet&      metricSet - metric set for which the calculation will be conducted
        //
        // Output:
        //     TCompletionCode - *CC_OK* means success
        //
        //////////////////////////////////////////////////////////////////////////////
        inline TCompletionCode ReadMetricsFromQueryReport( const uint8_t* rawReport, TTypedValue_1_0* outValues, CMetricSet& metricSet )
        {
            const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

            MD_CHECK_PTR_RET_A( adapterId, rawReport, CC_ERROR_INVALID_PARAMETER );
            MD_CHECK_PTR_RET_A( adapterId, outValues, CC_ERROR_INVALID_PARAMETER );

            m_gpuCoreClocks = 0;

            const uint32_t metricsCount = metricSet.GetParams()->MetricsCount;
            for( uint32_t i = 0; i < metricsCount; ++i )
            {
                outValues[i].ValueType   = VALUE_TYPE_UINT64;
                outValues[i].ValueUInt64 = 0ULL;

                auto metric = metricSet.GetMetricExplicit( i );
                MD_CHECK_PTR_RET_A( adapterId, metric, CC_ERROR_GENERAL );

                auto& metricParams = *metric->GetParams();

                if( metricParams.QueryReadEquation )
                {
                    outValues[i] = CalculateReadEquation( static_cast<CEquation&>( *( metricParams.QueryReadEquation ) ), rawReport );
                }

                if( m_gpuCoreClocks == 0 && std::string_view( metricParams.SymbolName ) == "GpuCoreClocks" )
                {
                    m_gpuCoreClocks = outValues[i].ValueUInt64;
                }
            }

            return CC_OK;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     ReadMetricsFromIoReport
        //
        // Description:
        //     Reads metrics from a given metric set using raw report data for prev and last report.
        //
        // Input:
        //     const uint8_t*   rawRaportLast - (IN) last (next) single raw report
        //     const uint8_t*   rawRaportPrev - (IN) previous single raw report
        //     TTypedValue_1_0* outValues     - (OUT) read metric values
        //     CMetricSet&      metricSet     - MetricSet for calculations
        //
        // Output:
        //     TCompletionCode - *CC_OK* means success
        //
        //////////////////////////////////////////////////////////////////////////////
        inline TCompletionCode ReadMetricsFromIoReport( const uint8_t* rawRaportLast, const uint8_t* rawRaportPrev, TTypedValue_1_0* outValues, CMetricSet& metricSet )
        {
            const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

            MD_CHECK_PTR_RET_A( adapterId, rawRaportLast, CC_ERROR_INVALID_PARAMETER );
            MD_CHECK_PTR_RET_A( adapterId, rawRaportPrev, CC_ERROR_INVALID_PARAMETER );
            MD_CHECK_PTR_RET_A( adapterId, outValues, CC_ERROR_INVALID_PARAMETER );

            m_gpuCoreClocks = 0;

            const uint32_t metricsCount = metricSet.GetParams()->MetricsCount;
            for( uint32_t i = 0; i < metricsCount; ++i )
            {
                outValues[i].ValueType   = VALUE_TYPE_UINT64;
                outValues[i].ValueUInt64 = 0ULL;

                auto metric = metricSet.GetMetricExplicit( i );
                MD_CHECK_PTR_RET_A( adapterId, metric, CC_ERROR_GENERAL );

                auto& metricParams = *metric->GetParams();

                if( metricParams.IoReadEquation )
                {
                    outValues[i] = CalculateReadEquationAndDelta( static_cast<CEquation&>( *( metricParams.IoReadEquation ) ), metricParams.DeltaFunction, rawRaportLast, rawRaportPrev );
                }

                if( m_gpuCoreClocks == 0 && std::string_view( metricParams.SymbolName ) == "GpuCoreClocks" )
                {
                    m_gpuCoreClocks = outValues[i].ValueUInt64;
                }
            }

            return CC_OK;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     NormalizeMetrics
        //
        // Description:
        //     Normalizes metrics from a given metric set using previously read data.
        //
        // Input:
        //     TTypedValue_1_0* deltaValues - (IN) previously read metric delta values
        //     TTypedValue_1_0* outValues   - (OUT) output normalized metric values
        //     CMetricSet&      metricSet   - MetricSet for calculations
        //
        //////////////////////////////////////////////////////////////////////////////
        inline void NormalizeMetrics( TTypedValue_1_0* deltaValues, TTypedValue_1_0* outValues, CMetricSet& metricSet )
        {
            const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

            if( !deltaValues || !outValues )
            {
                MD_ASSERT_A( adapterId, deltaValues != nullptr );
                MD_ASSERT_A( adapterId, outValues != nullptr );
                MD_LOG_A( adapterId, LOG_ERROR, "error: nullptr params" );
                return;
            }

            const uint32_t metricsCount = metricSet.GetParams()->MetricsCount;
            for( uint32_t i = 0; i < metricsCount; ++i )
            {
                auto metric = metricSet.GetMetricExplicit( i );
                MD_CHECK_PTR_RET_A( adapterId, metric, MD_EMPTY );

                auto& metricParams = *metric->GetParams();

                outValues[i] = metricParams.NormEquation
                    ? CalculateLocalNormalizationEquation( static_cast<CEquation&>( *( metricParams.NormEquation ) ), deltaValues, outValues, i )
                    : deltaValues[i];

                switch( metricParams.ResultType )
                {
                    case RESULT_UINT32:
                        if( outValues[i].ValueType != VALUE_TYPE_UINT32 )
                        {
                            outValues[i].ValueUInt32 = CastToUInt32( outValues[i] );
                            outValues[i].ValueType   = VALUE_TYPE_UINT32;
                        }
                        break;

                    case RESULT_UINT64:
                        if( outValues[i].ValueType != VALUE_TYPE_UINT64 )
                        {
                            outValues[i].ValueUInt64 = CastToUInt64( outValues[i] );
                            outValues[i].ValueType   = VALUE_TYPE_UINT64;
                        }
                        break;

                    case RESULT_FLOAT:
                        if( outValues[i].ValueType != VALUE_TYPE_FLOAT )
                        {
                            outValues[i].ValueFloat = CastToFloat( outValues[i] );
                            outValues[i].ValueType  = VALUE_TYPE_FLOAT;
                        }
                        break;

                    case RESULT_BOOL:
                        if( outValues[i].ValueType != VALUE_TYPE_BOOL )
                        {
                            outValues[i].ValueBool = CastToBoolean( outValues[i] );
                            outValues[i].ValueType = VALUE_TYPE_BOOL;
                        }
                        break;

                    default:
                        MD_ASSERT_A( adapterId, false );
                }
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     ReadInformation
        //
        // Description:
        //     Reads information from a given metric set.
        //
        // Input:
        //     const uint8_t*   rawData      - (IN) single raw report data
        //     TTypedValue_1_0* outValues    - (OUT) out values with calculated information
        //     CMetricSet&      metricSet    - MetricSet for calculations
        //     int32_t          contextIdIdx - index of contextId information to cache the value
        //
        //////////////////////////////////////////////////////////////////////////////
        inline void ReadInformation( const uint8_t* rawData, TTypedValue_1_0* outValues, CMetricSet& metricSet, int32_t contextIdIdx )
        {
            if( !rawData || !outValues )
            {
                const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

                MD_ASSERT_A( adapterId, rawData != nullptr );
                MD_ASSERT_A( adapterId, outValues != nullptr );
                MD_LOG_A( adapterId, LOG_ERROR, "error: nullptr params" );
                return;
            }

            auto&          metricSetParams  = *metricSet.GetParams();
            const uint32_t apiMask          = metricSetParams.ApiMask;
            const uint32_t informationCount = metricSetParams.InformationCount;

            for( uint32_t i = 0; i < informationCount; ++i )
            {
                auto information = metricSet.GetInformation( i );

                ReadSingleInformation( rawData, information, apiMask, &outValues[i] );
            }

            if( contextIdIdx != -1 )
            {
                // Value stored to handle PreviousContextId information and context filtering
                m_contextIdPrev = outValues[contextIdIdx].ValueUInt64;
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     ReadContextIdInformation
        //
        // Description:
        //     Reads contextId information to store it.
        //     Done only in Stream.
        //
        // Input:
        //     const uint8_t* rawData      - (IN) single raw report data
        //     CMetricSet&    metricSet    - MetricSet for calculations
        //     int32_t        contextIdIdx - index of contextId information
        //
        //////////////////////////////////////////////////////////////////////////////
        inline void ReadContextIdInformation( const uint8_t* rawData, CMetricSet& metricSet, int32_t contextIdIdx )
        {
            if( contextIdIdx == -1 )
            {
                m_contextIdPrev = 0;
                return;
            }

            TTypedValue_1_0   outValue    = {};
            IInformation_1_0* information = metricSet.GetInformation( static_cast<uint32_t>( contextIdIdx ) );
            const uint32_t    apiMask     = metricSet.GetParams()->ApiMask;

            ReadSingleInformation( rawData, information, apiMask, &outValue );

            m_contextIdPrev = outValue.ValueUInt64;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     ReadInformationByIndex
        //
        // Description:
        //     Reads information by given index as uint64_t.
        //
        // Input:
        //     const uint8_t* rawData        - (IN) single raw report data
        //     CMetricSet&    metricSet      - MetricSet for calculations
        //     int32_t        informationIdx - index of information
        //
        // Output:
        //     uint64_t - Information value in uint64_t format
        //
        //////////////////////////////////////////////////////////////////////////////
        inline uint64_t ReadInformationByIndex( const uint8_t* rawData, CMetricSet& metricSet, int32_t informationIndex )
        {
            if( informationIndex == -1 )
            {
                return 0;
            }

            TTypedValue_1_0   outValue    = {};
            IInformation_1_0* information = metricSet.GetInformation( static_cast<uint32_t>( informationIndex ) );
            const uint32_t    apiMask     = metricSet.GetParams()->ApiMask;

            ReadSingleInformation( rawData, information, apiMask, &outValue );

            return outValue.ValueUInt64;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     ReadSingleInformation
        //
        // Description:
        //     Reads single information.
        //
        // Input:
        //     const uint8_t*    rawReport   - single raw report
        //     IInformation_1_0* information - information to calculate
        //     uint32_t          apiMask     - API mask (needed for choosing proper equation)
        //     TTypedValue_1_0*  outValue    - (OUT) read information value
        //
        //////////////////////////////////////////////////////////////////////////////
        inline void ReadSingleInformation( const uint8_t* rawReport, IInformation_1_0* information, uint32_t apiMask, TTypedValue_1_0* outValue )
        {
            if( !rawReport || !information || !outValue )
            {
                const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

                MD_ASSERT_A( adapterId, rawReport != nullptr );
                MD_ASSERT_A( adapterId, information != nullptr );
                MD_ASSERT_A( adapterId, outValue != nullptr );
                MD_LOG_A( adapterId, LOG_ERROR, "error: nullptr params" );
                return;
            }

            constexpr uint32_t streamMask = API_TYPE_IOSTREAM;

            auto& informationParams = *information->GetParams();
            auto  equation          = ( apiMask & streamMask )
                          ? informationParams.IoReadEquation
                          : informationParams.QueryReadEquation;

            if( equation != nullptr )
            {
                *outValue = CalculateReadEquation( static_cast<CEquation&>( *equation ), rawReport );
            }
            else
            {
                outValue->ValueUInt64 = 0ULL;
            }

            outValue->ValueType = ( informationParams.InfoType == INFORMATION_TYPE_FLAG )
                ? VALUE_TYPE_BOOL
                : VALUE_TYPE_UINT64;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     ReadIoMeasurementInformation
        //
        // Description:
        //     Calculates IoMeasurementInformation obtained on every ReadIoStream.
        //
        // Input:
        //     IConcurrentGroup_1_1& concurrentGroup - concurrentGroup which was used during ReadIoStream
        //     TTypedValue_1_0*      outValues       - (OUT) calculated values
        //
        //////////////////////////////////////////////////////////////////////////////
        inline void ReadIoMeasurementInformation( IConcurrentGroup_1_1& concurrentGroup, TTypedValue_1_0* outValues )
        {
            const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

            if( !outValues )
            {
                MD_ASSERT_A( adapterId, outValues != nullptr );
                MD_LOG_A( adapterId, LOG_ERROR, "ERROR: outValues is nullptr" );
                return;
            }

            for( uint32_t i = 0; i < concurrentGroup.GetParams()->IoMeasurementInformationCount; ++i )
            {
                auto measurementInfo = concurrentGroup.GetIoMeasurementInformation( i );
                MD_ASSERT_A( adapterId, measurementInfo != nullptr );

                auto& measurementInfoParams = *measurementInfo->GetParams();

                if( auto equation = measurementInfoParams.IoReadEquation;
                    equation )
                {
                    outValues[i] = CalculateReadEquation( static_cast<CEquation&>( *equation ), nullptr );
                }
                else
                {
                    outValues[i].ValueUInt64 = 0ULL;
                }

                outValues[i].ValueType = ( measurementInfoParams.InfoType == INFORMATION_TYPE_FLAG )
                    ? VALUE_TYPE_BOOL
                    : VALUE_TYPE_UINT64;
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CalculateMaxValues
        //
        // Description:
        //     Calculates max value for every metric, specified by MaxValueEquation.
        //     If the equation isn't present, current normalized metric value is used as max value.
        //
        // Input:
        //     TTypedValue_1_0* deltaMetricValues - (IN) previously read metric delta values
        //     TTypedValue_1_0* outMetricValues   - (IN) normalized metric values
        //     TTypedValue_1_0* outMaxValues      - (OUT) output max values
        //     CMetricSet&      metricSet         - MetricSet for calculations
        //
        //////////////////////////////////////////////////////////////////////////////
        inline void CalculateMaxValues( TTypedValue_1_0* deltaMetricValues, TTypedValue_1_0* outMetricValues, TTypedValue_1_0* outMaxValues, CMetricSet& metricSet )
        {
            const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

            if( !deltaMetricValues || !outMetricValues || !outMaxValues )
            {
                MD_ASSERT_A( adapterId, deltaMetricValues != nullptr );
                MD_ASSERT_A( adapterId, outMetricValues != nullptr );
                MD_ASSERT_A( adapterId, outMaxValues != nullptr );
                MD_LOG_A( adapterId, LOG_ERROR, "error: nullptr params" );
                return;
            }

            const uint32_t metricsCount = metricSet.GetParams()->MetricsCount;
            for( uint32_t i = 0; i < metricsCount; ++i )
            {
                auto metric = metricSet.GetMetricExplicit( i );
                MD_CHECK_PTR_RET_A( adapterId, metric, MD_EMPTY );

                auto& metricParams = *metric->GetParams();

                outMaxValues[i] = metricParams.MaxValueEquation
                    ? CalculateLocalNormalizationEquation( static_cast<CEquation&>( *( metricParams.MaxValueEquation ) ), deltaMetricValues, outMetricValues, i )
                    : outMetricValues[i];
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //    CMetricsCalculator
        //
        // Method:
        //     SaveReport
        //
        // Description:
        //     Stores report for next calculations
        //
        // Input:
        //     const uint8_t* reportToSave - (IN) single raw report to save
        //
        // Output:
        //     TCompletionCode - CC_OK on success
        //
        //////////////////////////////////////////////////////////////////////////////
        inline TCompletionCode SaveReport( const uint8_t* reportToSave )
        {
            const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

            MD_CHECK_PTR_RET_A( adapterId, m_savedReport, CC_ERROR_INVALID_PARAMETER );
            MD_CHECK_PTR_RET_A( adapterId, reportToSave, CC_ERROR_INVALID_PARAMETER );

            bool res = iu_memcpy_s( m_savedReport, m_savedReportSize, reportToSave, m_savedReportSize );
            if( res )
            {
                m_savedReportPresent = true;
            }

            return res ? CC_OK : CC_ERROR_GENERAL;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //    CMetricsCalculator
        //
        // Method:
        //     SaveCalculatedReport
        //
        // Description:
        //     Stores calculated report for next calculations
        //
        // Input:
        //     const TTypedValue_1_0* reportToSave - (IN) single calculated report to save
        //
        // Output:
        //     TCompletionCode - CC_OK on success
        //
        //////////////////////////////////////////////////////////////////////////////
        inline TCompletionCode SaveCalculatedReport( const TTypedValue_1_0* reportToSave )
        {
            const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

            MD_CHECK_PTR_RET_A( adapterId, reportToSave, CC_ERROR_INVALID_PARAMETER );
            MD_CHECK_PTR_RET_A( adapterId, m_prevValues, CC_ERROR_INVALID_PARAMETER );

            const uint32_t reportSize = m_prevValuesCount * sizeof( TTypedValue_1_0 );

            return iu_memcpy_s( m_prevValues, reportSize, reportToSave, reportSize ) ? CC_OK : CC_ERROR_GENERAL;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //    CMetricsCalculator
        //
        // Method:
        //     SavedReportPresent
        //
        // Description:
        //     Check if Calculator has saved report.
        //
        // Output:
        //     bool - true if report saved
        //
        //////////////////////////////////////////////////////////////////////////////
        inline bool SavedReportPresent()
        {
            return m_savedReportPresent;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //    CMetricsCalculator
        //
        // Method:
        //     GetSavedReport
        //
        // Description:
        //     Getter for saved report pointer.
        //
        // Output:
        //     uint8_t* - saved report
        //
        //////////////////////////////////////////////////////////////////////////////
        inline uint8_t* GetSavedReport()
        {
            return m_savedReport;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //    CMetricsCalculator
        //
        // Method:
        //     DiscardSavedReport
        //
        // Description:
        //     Sets to false flag indicating report save.
        //
        //////////////////////////////////////////////////////////////////////////////
        inline void DiscardSavedReport()
        {
            m_savedReportPresent = false;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CastToUInt32
        //
        // Description:
        //     Returns typed value casted to uint32.
        //
        // Input:
        //     const TTypedValue_1_0& value - typed value to be casted
        //
        // Output:
        //     uint32_t                     - casted typed value to uint32
        //
        //////////////////////////////////////////////////////////////////////////////
        inline uint32_t CastToUInt32( const TTypedValue_1_0& value )
        {
            switch( value.ValueType )
            {
                case VALUE_TYPE_BOOL:
                    return ( value.ValueBool ) ? 1U : 0U;

                case VALUE_TYPE_UINT32:
                    return value.ValueUInt32;

                case VALUE_TYPE_UINT64:
                    return static_cast<uint32_t>( value.ValueUInt64 );

                case VALUE_TYPE_FLOAT:
                    return static_cast<uint32_t>( value.ValueFloat );

                default:
                    return 0U;
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CastToUInt64
        //
        // Description:
        //     Returns typed value casted to uint64.
        //
        // Input:
        //     const TTypedValue_1_0& value - typed value to be casted
        //
        // Output:
        //     uint64_t                     - casted typed value to uint64
        //
        //////////////////////////////////////////////////////////////////////////////
        inline uint64_t CastToUInt64( const TTypedValue_1_0& value )
        {
            switch( value.ValueType )
            {
                case VALUE_TYPE_BOOL:
                    return ( value.ValueBool ) ? 1ULL : 0ULL;

                case VALUE_TYPE_UINT32:
                    return static_cast<uint64_t>( value.ValueUInt32 );

                case VALUE_TYPE_UINT64:
                    return value.ValueUInt64;

                case VALUE_TYPE_FLOAT:
                    return static_cast<uint64_t>( value.ValueFloat );

                default:
                    return 0ULL;
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CastToFloat
        //
        // Description:
        //     Returns typed value casted to float.
        //
        // Input:
        //     const TTypedValue_1_0& value - typed value to be casted
        //
        // Output:
        //     float                        - casted typed value to float
        //
        //////////////////////////////////////////////////////////////////////////////
        inline float CastToFloat( const TTypedValue_1_0& value )
        {
            switch( value.ValueType )
            {
                case VALUE_TYPE_BOOL:
                    return ( value.ValueBool ) ? 1.0f : 0.0f;

                case VALUE_TYPE_UINT32:
                    return static_cast<float>( value.ValueUInt32 );

                case VALUE_TYPE_UINT64:
                    return static_cast<float>( value.ValueUInt64 );

                case VALUE_TYPE_FLOAT:
                    return value.ValueFloat;

                default:
                    return 0.0f;
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CastToBoolean
        //
        // Description:
        //     Returns typed value casted to boolean.
        //
        // Input:
        //     const TTypedValue_1_0& value - typed value to be casted
        //
        // Output:
        //     bool                         - casted typed value to boolean
        //
        //////////////////////////////////////////////////////////////////////////////
        inline bool CastToBoolean( const TTypedValue_1_0& value )
        {
            switch( value.ValueType )
            {
                case VALUE_TYPE_BOOL:
                    return value.ValueBool;

                case VALUE_TYPE_UINT32:
                    return value.ValueUInt32 != 0U;

                case VALUE_TYPE_UINT64:
                    return value.ValueUInt64 != 0ULL;

                case VALUE_TYPE_FLOAT:
                    return value.ValueFloat != 0.0f;

                default:
                    return false;
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     ReadBitfield
        //
        // Description:
        //     Returns bitfield from the report.
        //
        // Input:
        //     const uint8_t* rawReport - raw report
        //     uint32_t       bitOffset - bit offset
        //     uint32_t       bitCount  - bit count
        //
        // Output:
        //     uint64_t                 - bitfield from the report
        //
        //////////////////////////////////////////////////////////////////////////////
        inline uint64_t ReadBitfield(
            const uint8_t* rawReport,
            uint32_t       bitOffset,
            uint32_t       bitCount )
        {
            if( !rawReport || ( bitCount > 32 ) || ( bitCount == 0 ) || ( bitCount + bitOffset > 32 ) )
            {
                const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

                MD_ASSERT_A( adapterId, false );
                MD_LOG_A( adapterId, LOG_ERROR, "error: invalid params" );
                return 0;
            }

            // Build mask
            const uint32_t mask = MD_BITMASK_RANGE( bitOffset, bitOffset + bitCount - 1 );

            // Get integer in the way in is alignment safe
            const uint32_t data = *reinterpret_cast<const uint32_t*>( rawReport );

            return static_cast<uint64_t>( ( data & mask ) >> bitOffset );
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     GetMetricsDevice
        //
        // Description:
        //     Returns metrics device
        //
        // Output:
        //     CMetricsDevice& - metrics device
        //
        //////////////////////////////////////////////////////////////////////////////
        inline CMetricsDevice& GetMetricsDevice()
        {
            return m_device;
        }

    private:
        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CalculateDeltaFunction
        //
        // Description:
        //     Calculates the given delta function.
        //
        // Input:
        //     TDeltaFunction_1_0     deltaFunction  - delta function to be calculated
        //     const TTypedValue_1_0& lastValue     - (IN) last (next) value
        //     const TTypedValue_1_0& previousValue - (IN) previous value
        //
        // Output:
        //     TTypedValue_1_0 - output calculated delta value
        //
        //////////////////////////////////////////////////////////////////////////////
        inline TTypedValue_1_0 CalculateDeltaFunction(
            TDeltaFunction_1_0     deltaFunction,
            const TTypedValue_1_0& lastValue,
            const TTypedValue_1_0& previousValue )
        {
            TTypedValue_1_0 typedValue = {};

            switch( deltaFunction.FunctionType )
            {
                case DELTA_BOOL_OR:
                    typedValue.ValueUInt64 = ( ( lastValue.ValueUInt64 | previousValue.ValueUInt64 ) != 0 ) ? 1ULL : 0ULL;
                    typedValue.ValueType   = VALUE_TYPE_UINT64;
                    return typedValue;

                case DELTA_BOOL_XOR:
                    typedValue.ValueUInt64 = ( ( lastValue.ValueUInt64 ^ previousValue.ValueUInt64 ) != 0 ) ? 1ULL : 0ULL;
                    typedValue.ValueType   = VALUE_TYPE_UINT64;
                    return typedValue;

                case DELTA_GET_LAST:
                    return lastValue;

                case DELTA_GET_PREVIOUS:
                    return previousValue;

                case DELTA_NS_TIME:
                    // No 'break' intentional - NS_TIME should be used only for overflow functions, here use as DELTA 32 or DELTA 56
                    switch( m_device.GetPlatformIndex() )
                    {
                        case GENERATION_BMG:
                        case GENERATION_LNL:
                        case GENERATION_PTL:
                            deltaFunction.BitsCount = 56;
                            break;
                        default:
                            deltaFunction.BitsCount = 32;
                            break;
                    }
                    [[fallthrough]];

                case DELTA_N_BITS:
                    if( deltaFunction.BitsCount <= 64 )
                    {
                        if( previousValue.ValueUInt64 > lastValue.ValueUInt64 )
                        {
                            if( deltaFunction.BitsCount < 64 )
                            {
                                const uint64_t value   = lastValue.ValueUInt64 | ( 1ULL << deltaFunction.BitsCount );
                                typedValue.ValueUInt64 = value - previousValue.ValueUInt64;
                            }
                            else
                            {
                                typedValue.ValueUInt64 = ( std::numeric_limits<uint64_t>::max )() - previousValue.ValueUInt64 + lastValue.ValueUInt64;
                            }

                            typedValue.ValueType = VALUE_TYPE_UINT64;
                            return typedValue;
                        }
                        else
                        {
                            typedValue.ValueUInt64 = lastValue.ValueUInt64 - previousValue.ValueUInt64;
                            typedValue.ValueType   = VALUE_TYPE_UINT64;
                            return typedValue;
                        }
                    }
                    break;

                case DELTA_FUNCTION_NULL:
                    typedValue.ValueUInt64 = 0;
                    typedValue.ValueType   = VALUE_TYPE_UINT64;
                    return typedValue;

                default:
                    MD_ASSERT_A( m_device.GetAdapter().GetAdapterId(), false );
                    break;
            }

            typedValue.ValueUInt64 = 0;
            typedValue.ValueType   = VALUE_TYPE_UINT64;

            return typedValue;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     GetGlobalSymbolValue
        //
        // Description:
        //     Returns global symbol of a given name. Uses MetricsDevice.
        //
        // Input:
        //     const char* symbolName - global symbol name
        //
        // Output:
        //     TTypedValue_1_0* - global symbol typed value, null if error
        //
        //////////////////////////////////////////////////////////////////////////////
        inline TTypedValue_1_0* GetGlobalSymbolValue( const char* symbolName )
        {
            const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();
            MD_CHECK_PTR_RET_A( adapterId, symbolName, nullptr );

            return m_device.GetGlobalSymbolValueByName( symbolName );
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CalculateReadEquation
        //
        // Description:
        //     Calculates the given read equation.
        //
        // Input:
        //     IEquation_1_0* equation  - read equation to calculate
        //     const uint8_t* rawReport - (IN) single raw report
        //
        // Output:
        //     TTypedValue_1_0 - output read value
        //
        //////////////////////////////////////////////////////////////////////////////
        inline TTypedValue_1_0 CalculateReadEquation(
            CEquation&     equation,
            const uint8_t* rawReport )
        {
            const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

            TTypedValue_1_0 typedValue     = {};
            bool            isValid        = true;
            uint32_t        algorithmCheck = 0;

            ClearStack( m_readEquationStack );
            const auto&    equationElements      = equation.GetElementsVector();
            const uint32_t equationElementsCount = static_cast<uint32_t>( equationElements.size() );
            for( uint32_t i = 0; i < equationElementsCount && isValid; ++i )
            {
                const auto& element = equationElements[i];
                switch( element.Type )
                {
                    case EQUATION_ELEM_RD_BITFIELD:
                        typedValue.ValueUInt64 = ReadBitfield( (const uint8_t*) ( rawReport + element.ReadParams.ByteOffset ), element.ReadParams.BitOffset, element.ReadParams.BitsCount );
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_RD_UINT8:
                    {
                        uint8_t byteValue      = *( (const uint8_t*) ( rawReport + element.ReadParams.ByteOffset ) );
                        typedValue.ValueUInt64 = (uint64_t) byteValue;
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_RD_UINT16:
                    {
                        uint16_t shortValue    = *( (const uint16_t*) ( rawReport + element.ReadParams.ByteOffset ) );
                        typedValue.ValueUInt64 = (uint64_t) shortValue;
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_RD_UINT32:
                    {
                        uint32_t dwordValue    = *( (const uint32_t*) ( rawReport + element.ReadParams.ByteOffset ) );
                        typedValue.ValueUInt64 = (uint64_t) dwordValue;
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_RD_UINT64:
                        typedValue.ValueUInt64 = *( (const uint64_t*) ( rawReport + element.ReadParams.ByteOffset ) );
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_RD_FLOAT:
                        typedValue.ValueFloat = *( (const float*) ( rawReport + element.ReadParams.ByteOffset ) );
                        typedValue.ValueType  = VALUE_TYPE_FLOAT;
                        isValid               = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_RD_40BIT_CNTR:
                    {
                        TLargeInteger largeValue;
                        largeValue.u.LowPart   = *( (const uint32_t*) ( rawReport + element.ReadParams.ByteOffset ) );
                        largeValue.u.HighPart  = ( int32_t ) * ( (const uint8_t*) ( rawReport + element.ReadParams.ByteOffsetExt ) );
                        typedValue.ValueUInt64 = static_cast<uint64_t>( largeValue.QuadPart );
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_IMM_UINT64:
                        typedValue.ValueUInt64 = element.ImmediateUInt64;
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_IMM_FLOAT:
                        typedValue.ValueFloat = element.ImmediateFloat;
                        typedValue.ValueType  = VALUE_TYPE_FLOAT;
                        isValid               = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_GLOBAL_SYMBOL:
                    {
                        TTypedValue_1_0* pValue = GetGlobalSymbolValue( element.SymbolName );
                        if( pValue )
                        {
                            typedValue = *pValue;
                        }
                        else
                        {
                            typedValue.ValueUInt64 = 0;
                            typedValue.ValueType   = VALUE_TYPE_UINT64;
                        }
                        isValid = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_INFORMATION_SYMBOL:
                    {
                        if( std::string_view( element.SymbolName ) == "PreviousContextId" )
                        {
                            // Return cached context ID from the previous report
                            typedValue.ValueUInt64 = m_contextIdPrev;
                        }
                        else
                        {
                            // TODO: not supported yet
                            typedValue.ValueUInt64 = 0;
                            MD_ASSERT_A( adapterId, false );
                        }
                        typedValue.ValueType = VALUE_TYPE_UINT64;
                        isValid              = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_OPERATION:
                    {
                        // Pop two values from stack
                        TTypedValue_1_0 valueLast = m_readEquationStack.top();
                        m_readEquationStack.pop();
                        algorithmCheck--;
                        TTypedValue_1_0 valuePrev = m_readEquationStack.top();
                        m_readEquationStack.pop();
                        algorithmCheck--;

                        typedValue = CalculateEquationElemOperation( element.Operation, valuePrev, valueLast );
                        isValid    = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_LOCAL_COUNTER_SYMBOL:
                        if( m_gpuCoreClocks != 0 && std::string_view( element.SymbolName ) == "GpuCoreClocks" )
                        {
                            typedValue.ValueUInt64 = m_gpuCoreClocks;
                            typedValue.ValueType   = VALUE_TYPE_UINT64;
                            isValid                = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );
                            break;
                        }

                        typedValue.ValueUInt64 = 0LL;
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_readEquationStack, typedValue, algorithmCheck );

                        if( IsPlatformMatch( m_device.GetPlatformIndex(), GENERATION_ACM, GENERATION_PVC, GENERATION_MTL, GENERATION_ARL ) &&
                            strstr( element.SymbolName, "GtSlice" ) != nullptr )
                        {
                            // Exception for missing global symbols (GtSlice[X]XeCore[Y]) in read equations.
                            break;
                        }
                        // Asserts, because this is not a valid condition
                        [[fallthrough]];

                    default:
                        MD_ASSERT_A( adapterId, false );
                        break;
                }
            }
            // Here should be only 1 element on the list - the result (if the equation is fine)
            MD_ASSERT_A( adapterId, algorithmCheck == 1 );

            if( isValid && algorithmCheck == 1 )
            {
                typedValue = m_readEquationStack.top();
                m_readEquationStack.pop();
            }
            else
            {
                typedValue.ValueUInt64 = 0;
                typedValue.ValueType   = VALUE_TYPE_UINT64;
            }

            return typedValue;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CalculateReadEquationAndDelta
        //
        // Description:
        //     Calculates the given read equation using delta function directly after reading
        //     raw offsets.
        //
        // Input:
        //     IEquation_1_0*     equation       - read equation to calculate
        //     TDeltaFunction_1_0 deltaFunction  - delta function to use during calculations
        //     const uint8_t*     pRawReportLast - (IN) last (next) single raw report
        //     const uint8_t*     pRawReportPrev - (IN) previous single raw report
        //
        // Output:
        //     TTypedValue_1_0 - output read value
        //
        //////////////////////////////////////////////////////////////////////////////
        inline TTypedValue_1_0 CalculateReadEquationAndDelta(
            CEquation&         equation,
            TDeltaFunction_1_0 deltaFunction,
            const uint8_t*     pRawReportLast,
            const uint8_t*     pRawReportPrev )
        {
            TTypedValue_1_0    typedValue, typedValuePrev, typedValueLast;
            TDeltaFunction_1_0 readDeltaFunction;
            // As we calculate delta when reading operands DELTA_NS_TIME works as a normal DELTA_32 or DELTA_56
            if( deltaFunction.FunctionType == DELTA_NS_TIME )
            {
                readDeltaFunction.FunctionType = DELTA_N_BITS;

                switch( m_device.GetPlatformIndex() )
                {
                    case GENERATION_BMG:
                    case GENERATION_LNL:
                    case GENERATION_PTL:
                        readDeltaFunction.BitsCount = 56;
                        break;
                    default:
                        readDeltaFunction.BitsCount = 32;
                        break;
                }
            }
            else
            {
                readDeltaFunction = deltaFunction;
            }

            const uint32_t adapterId      = m_device.GetAdapter().GetAdapterId();
            bool           isValid        = true;
            uint32_t       algorithmCheck = 0;

            ClearStack( m_readEquationAndDeltaStack );
            const auto&    equationElements      = equation.GetElementsVector();
            const uint32_t equationElementsCount = static_cast<uint32_t>( equationElements.size() );
            for( uint32_t i = 0; i < equationElementsCount && isValid; ++i )
            {
                const auto& element = equationElements[i];
                switch( element.Type )
                {
                    case EQUATION_ELEM_RD_BITFIELD:

                        typedValuePrev.ValueUInt64 = ReadBitfield( pRawReportPrev + element.ReadParams.ByteOffset, element.ReadParams.BitOffset, element.ReadParams.BitsCount );
                        typedValuePrev.ValueType   = VALUE_TYPE_UINT64;

                        typedValueLast.ValueUInt64 = ReadBitfield( pRawReportLast + element.ReadParams.ByteOffset, element.ReadParams.BitOffset, element.ReadParams.BitsCount );
                        typedValueLast.ValueType   = VALUE_TYPE_UINT64;

                        typedValue = CalculateDeltaFunction( readDeltaFunction, typedValueLast, typedValuePrev );
                        isValid    = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_RD_UINT8:
                        typedValuePrev.ValueUInt64 =
                            ( uint64_t ) * ( (const uint8_t*) ( pRawReportPrev + element.ReadParams.ByteOffset ) );
                        typedValuePrev.ValueType = VALUE_TYPE_UINT64;

                        typedValueLast.ValueUInt64 =
                            ( uint64_t ) * ( (const uint8_t*) ( pRawReportLast + element.ReadParams.ByteOffset ) );
                        typedValueLast.ValueType = VALUE_TYPE_UINT64;

                        typedValue = CalculateDeltaFunction( readDeltaFunction, typedValueLast, typedValuePrev );
                        isValid    = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_RD_UINT16:
                        typedValuePrev.ValueUInt64 =
                            ( uint64_t ) * ( (const uint16_t*) ( pRawReportPrev + element.ReadParams.ByteOffset ) );
                        typedValuePrev.ValueType = VALUE_TYPE_UINT64;

                        typedValueLast.ValueUInt64 =
                            ( uint64_t ) * ( (const uint16_t*) ( pRawReportLast + element.ReadParams.ByteOffset ) );
                        typedValueLast.ValueType = VALUE_TYPE_UINT64;

                        typedValue = CalculateDeltaFunction( readDeltaFunction, typedValueLast, typedValuePrev );
                        isValid    = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_RD_UINT32:
                        typedValuePrev.ValueUInt64 =
                            ( uint64_t ) * ( (const uint32_t*) ( pRawReportPrev + element.ReadParams.ByteOffset ) );
                        typedValuePrev.ValueType = VALUE_TYPE_UINT64;

                        typedValueLast.ValueUInt64 =
                            ( uint64_t ) * ( (const uint32_t*) ( pRawReportLast + element.ReadParams.ByteOffset ) );
                        typedValueLast.ValueType = VALUE_TYPE_UINT64;

                        typedValue = CalculateDeltaFunction( readDeltaFunction, typedValueLast, typedValuePrev );
                        isValid    = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_RD_UINT64:
                        typedValuePrev.ValueUInt64 = *( (const uint64_t*) ( pRawReportPrev + element.ReadParams.ByteOffset ) );
                        typedValuePrev.ValueType   = VALUE_TYPE_UINT64;

                        typedValueLast.ValueUInt64 = *( (const uint64_t*) ( pRawReportLast + element.ReadParams.ByteOffset ) );
                        typedValueLast.ValueType   = VALUE_TYPE_UINT64;

                        typedValue = CalculateDeltaFunction( readDeltaFunction, typedValueLast, typedValuePrev );
                        isValid    = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_RD_FLOAT:
                        typedValuePrev.ValueFloat = *( (const float*) ( pRawReportPrev + element.ReadParams.ByteOffset ) );
                        typedValuePrev.ValueType  = VALUE_TYPE_FLOAT;

                        typedValueLast.ValueFloat = *( (const float*) ( pRawReportLast + element.ReadParams.ByteOffset ) );
                        typedValueLast.ValueType  = VALUE_TYPE_FLOAT;

                        typedValue = CalculateDeltaFunction( readDeltaFunction, typedValueLast, typedValuePrev );
                        isValid    = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_RD_40BIT_CNTR:
                    {
                        TLargeInteger largeValue;
                        largeValue.u.LowPart       = *( (const uint32_t*) ( pRawReportPrev + element.ReadParams.ByteOffset ) );
                        largeValue.u.HighPart      = ( int32_t ) * ( (const uint8_t*) ( pRawReportPrev + element.ReadParams.ByteOffsetExt ) );
                        typedValuePrev.ValueUInt64 = static_cast<uint64_t>( largeValue.QuadPart );
                        typedValuePrev.ValueType   = VALUE_TYPE_UINT64;

                        largeValue.u.LowPart       = *( (const uint32_t*) ( pRawReportLast + element.ReadParams.ByteOffset ) );
                        largeValue.u.HighPart      = ( int32_t ) * ( (const uint8_t*) ( pRawReportLast + element.ReadParams.ByteOffsetExt ) );
                        typedValueLast.ValueUInt64 = static_cast<uint64_t>( largeValue.QuadPart );
                        typedValueLast.ValueType   = VALUE_TYPE_UINT64;

                        typedValue = CalculateDeltaFunction( readDeltaFunction, typedValueLast, typedValuePrev );
                        isValid    = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_IMM_UINT64:
                        typedValue.ValueUInt64 = element.ImmediateUInt64;
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_IMM_FLOAT:
                        typedValue.ValueFloat = element.ImmediateFloat;
                        typedValue.ValueType  = VALUE_TYPE_FLOAT;
                        isValid               = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_GLOBAL_SYMBOL:
                    {
                        TTypedValue_1_0* pValue = GetGlobalSymbolValue( element.SymbolName );
                        if( pValue )
                        {
                            typedValue = *pValue;
                        }
                        else
                        {
                            typedValue.ValueUInt64 = 0LL;
                            typedValue.ValueType   = VALUE_TYPE_UINT64;
                        }
                        isValid = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_OPERATION:
                    {
                        // Pop two values from stack
                        TTypedValue_1_0 valueLast = m_readEquationAndDeltaStack.top();
                        m_readEquationAndDeltaStack.pop();
                        algorithmCheck--;
                        TTypedValue_1_0 valuePrev = m_readEquationAndDeltaStack.top();
                        m_readEquationAndDeltaStack.pop();
                        algorithmCheck--;

                        typedValue = CalculateEquationElemOperation( element.Operation, valuePrev, valueLast );
                        isValid    = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_LOCAL_COUNTER_SYMBOL:
                        if( m_gpuCoreClocks != 0 && std::string_view( element.SymbolName ) == "GpuCoreClocks" )
                        {
                            typedValue.ValueUInt64 = m_gpuCoreClocks;
                            typedValue.ValueType   = VALUE_TYPE_UINT64;
                            isValid                = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );
                            break;
                        }

                        typedValue.ValueUInt64 = 0LL;
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_readEquationAndDeltaStack, typedValue, algorithmCheck );

                        if( IsPlatformMatch( m_device.GetPlatformIndex(), GENERATION_ACM, GENERATION_PVC, GENERATION_MTL, GENERATION_ARL ) &&
                            strstr( element.SymbolName, "GtSlice" ) != nullptr )
                        {
                            // Exception for missing global symbols (GtSlice[X]XeCore[Y]) in read equations.
                            break;
                        }
                        // Asserts, because this is not a valid condition
                        [[fallthrough]];

                    default:
                        MD_ASSERT_A( adapterId, false );
                        break;
                }
            }
            // here should be only 1 element on the list - the result (if the equation is fine)
            MD_ASSERT_A( adapterId, algorithmCheck == 1 );

            if( isValid && algorithmCheck == 1 )
            {
                typedValue = m_readEquationAndDeltaStack.top();
                m_readEquationAndDeltaStack.pop();
            }
            else
            {
                typedValue.ValueUInt64 = 0LL;
                typedValue.ValueType   = VALUE_TYPE_UINT64;
            }

            return typedValue;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CalculateLocalNormalizationEquation
        //
        // Description:
        //     Calculates the given normalization equation.
        //
        // Input:
        //     IEquation_1_0*   equation         - (IN) normalization equation to be calculated
        //     TTypedValue_1_0* deltaValues      - (IN) previously calculated / read delta values
        //     TTypedValue_1_0* outValues        - (IN) so far normalized values (metrics with lower indices)
        //     CMetricSet*      metricSet        - MetricSet for calculation
        //     uint32_t         currentMetricIdx - index of the currently calculated metric
        //
        // Output:
        //     TTypedValue_1_0 - output normalized value
        //
        //////////////////////////////////////////////////////////////////////////////
        inline TTypedValue_1_0 CalculateLocalNormalizationEquation(
            CEquation&       equation,
            TTypedValue_1_0* deltaValues,
            TTypedValue_1_0* outValues,
            uint32_t         metricIndex )
        {
            const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

            TTypedValue_1_0 typedValue     = {};
            bool            isValid        = true;
            uint32_t        algorithmCheck = 0;

            ClearStack( m_normalizationEquationStack );
            const auto&    equationElements      = equation.GetElementsVector();
            const uint32_t equationElementsCount = static_cast<uint32_t>( equationElements.size() );
            for( uint32_t i = 0; i < equationElementsCount && isValid; ++i )
            {
                const auto& element = equationElements[i];
                switch( element.Type )
                {
                    case EQUATION_ELEM_RD_BITFIELD:
                    case EQUATION_ELEM_RD_UINT8:
                    case EQUATION_ELEM_RD_UINT16:
                    case EQUATION_ELEM_RD_UINT32:
                    case EQUATION_ELEM_RD_UINT64:
                    case EQUATION_ELEM_RD_FLOAT:
                    case EQUATION_ELEM_RD_40BIT_CNTR:
                        // Not allowed in norm equation
                        break;

                    case EQUATION_ELEM_IMM_FLOAT:
                        typedValue.ValueFloat = element.ImmediateFloat;
                        typedValue.ValueType  = VALUE_TYPE_FLOAT;
                        isValid               = EquationStackPush( m_normalizationEquationStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_IMM_UINT64:
                        typedValue.ValueUInt64 = element.ImmediateUInt64;
                        typedValue.ValueType   = VALUE_TYPE_UINT64;
                        isValid                = EquationStackPush( m_normalizationEquationStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_SELF_COUNTER_VALUE:
                        // Get result of delta equation
                        typedValue = deltaValues[metricIndex];
                        isValid    = EquationStackPush( m_normalizationEquationStack, typedValue, algorithmCheck );
                        break;

                    case EQUATION_ELEM_LOCAL_COUNTER_SYMBOL:
                    {
                        // The index is higher than or equals 0 if the symbol name was found, otherwise it equals -1
                        if( equationElements[i].MetricIndexInternal >= 0 )
                        {
                            typedValue = deltaValues[equationElements[i].MetricIndexInternal];
                        }
                        else
                        {
                            typedValue.ValueUInt64 = 0;
                            typedValue.ValueType   = VALUE_TYPE_UINT64;
                        }

                        isValid = EquationStackPush( m_normalizationEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_LOCAL_METRIC_SYMBOL:
                    {
                        // The index is higher than or equals 0 if the symbol name was found, otherwise it equals -1
                        if( equationElements[i].MetricIndexInternal >= 0 )
                        {
                            typedValue = outValues[equationElements[i].MetricIndexInternal];
                        }
                        else
                        {
                            typedValue.ValueUInt64 = 0;
                            typedValue.ValueType   = VALUE_TYPE_UINT64;
                        }

                        isValid = EquationStackPush( m_normalizationEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_PREV_METRIC_SYMBOL:
                    {
                        // The index is higher than or equals 0 if the symbol name was found, otherwise it equals -1
                        if( m_prevValues && equationElements[i].MetricIndexInternal >= 0 )
                        {
                            typedValue = m_prevValues[equationElements[i].MetricIndexInternal];
                        }
                        else
                        {
                            typedValue.ValueUInt64 = 0;
                            typedValue.ValueType   = VALUE_TYPE_UINT64;
                        }

                        isValid = EquationStackPush( m_normalizationEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_GLOBAL_SYMBOL:
                    {
                        TTypedValue_1_0* pValue = GetGlobalSymbolValue( element.SymbolName );
                        if( pValue )
                        {
                            typedValue = *pValue;
                        }
                        else
                        {
                            typedValue.ValueUInt64 = 0;
                            typedValue.ValueType   = VALUE_TYPE_UINT64;
                        }
                        isValid = EquationStackPush( m_normalizationEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_OPERATION:
                    {
                        // pop two values from stack
                        TTypedValue_1_0 valueLast = m_normalizationEquationStack.top();
                        m_normalizationEquationStack.pop();

                        algorithmCheck--;
                        TTypedValue_1_0 valuePrev = m_normalizationEquationStack.top();
                        m_normalizationEquationStack.pop();
                        algorithmCheck--;

                        typedValue = CalculateEquationElemOperation( element.Operation, valuePrev, valueLast );
                        isValid    = EquationStackPush( m_normalizationEquationStack, typedValue, algorithmCheck );
                        break;
                    }

                    case EQUATION_ELEM_STD_NORM_GPU_DURATION:
                        // equation stack should be empty
                        MD_ASSERT_A( adapterId, algorithmCheck == 0 );

                        // compute $Self $gpuCoreClocks FDIV 100 FMUL
                        if( m_gpuCoreClocks != 0 )
                        {
                            const float self          = CastToFloat( deltaValues[metricIndex] );
                            const float gpuCoreClocks = static_cast<float>( m_gpuCoreClocks );

                            typedValue.ValueFloat = 100.0f * self / gpuCoreClocks;
                            typedValue.ValueType  = VALUE_TYPE_FLOAT;
                            return typedValue;
                        }
                        else
                        {
                            // Warning: GpuCoreClocks is 0
                            typedValue.ValueFloat = 0.0f;
                            typedValue.ValueType  = VALUE_TYPE_FLOAT;
                            return typedValue;
                        }

                    case EQUATION_ELEM_STD_NORM_EU_AGGR_DURATION:
                        // equation stack should be empty
                        MD_ASSERT_A( adapterId, algorithmCheck == 0 );
                        // m_euCoresCount is needed here
                        MD_ASSERT_A( adapterId, m_euCoresCount != 0 );

                        // compute $Self $gpuCoreClocks $EUsCount UMUL FDIV 100 FMUL
                        if( m_gpuCoreClocks != 0 && m_euCoresCount != 0 )
                        {
                            const float self          = CastToFloat( deltaValues[metricIndex] );
                            const float gpuCoreClocks = static_cast<float>( m_gpuCoreClocks * m_euCoresCount );

                            typedValue.ValueFloat = 100.0f * self / gpuCoreClocks;
                            typedValue.ValueType  = VALUE_TYPE_FLOAT;
                            return typedValue;
                        }
                        else
                        {
                            // Warning: GpuCoreClocks or euCoresCount is 0
                            typedValue.ValueFloat = 0.0f;
                            typedValue.ValueType  = VALUE_TYPE_FLOAT;
                            return typedValue;
                        }

                    default:
                        break;
                }
            }
            // here should be only 1 element on the list - the result (if the equation is fine)
            MD_ASSERT_A( adapterId, algorithmCheck == 1 );

            if( isValid && algorithmCheck == 1 )
            {
                typedValue = m_normalizationEquationStack.top();
                m_normalizationEquationStack.pop();
            }
            else
            {
                typedValue.ValueUInt64 = 0;
                typedValue.ValueType   = VALUE_TYPE_UINT64;
            }

            return typedValue;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     CalculateEquationElemOperation
        //
        // Description:
        //     Calculates the given equation operation.
        //
        // Input:
        //     TEquationOperation     operation - operation to be calculated
        //     const TTypedValue_1_0& valuePrev - (IN) previous value
        //     const TTypedValue_1_0& valueLast - (IN) last (next) value
        //
        // Output:
        //     TTypedValue_1_0 - output calculated value
        //
        //////////////////////////////////////////////////////////////////////////////
        inline TTypedValue_1_0 CalculateEquationElemOperation(
            TEquationOperation     operation,
            const TTypedValue_1_0& valuePrev,
            const TTypedValue_1_0& valueLast )
        {
            TTypedValue_1_0 value = {};
            value.ValueType       = VALUE_TYPE_UINT64;
            value.ValueUInt64     = 0ULL;

            switch( operation )
            {
                case EQUATION_OPER_AND:
                    value.ValueUInt64 = CastToUInt64( valuePrev ) & CastToUInt64( valueLast );
                    break;

                case EQUATION_OPER_OR:
                    value.ValueUInt64 = CastToUInt64( valuePrev ) | CastToUInt64( valueLast );
                    break;

                case EQUATION_OPER_RSHIFT:
                    value.ValueUInt64 = CastToUInt64( valuePrev ) >> CastToUInt64( valueLast );
                    break;

                case EQUATION_OPER_LSHIFT:
                    value.ValueUInt64 = CastToUInt64( valuePrev ) << CastToUInt64( valueLast );
                    break;

                case EQUATION_OPER_XOR:
                    value.ValueUInt64 = CastToUInt64( valuePrev ) ^ CastToUInt64( valueLast );
                    break;

                case EQUATION_OPER_XNOR:
                    value.ValueUInt64 = ~( CastToUInt64( valuePrev ) ^ CastToUInt64( valueLast ) );
                    break;

                case EQUATION_OPER_AND_L:
                    value.ValueBool = CastToUInt64( valuePrev ) && CastToUInt64( valueLast );
                    value.ValueType = VALUE_TYPE_BOOL;
                    break;

                case EQUATION_OPER_EQUALS:
                    value.ValueBool = CastToUInt64( valuePrev ) == CastToUInt64( valueLast );
                    value.ValueType = VALUE_TYPE_BOOL;
                    break;

                case EQUATION_OPER_UADD:
                    value.ValueUInt64 = CastToUInt64( valuePrev ) + CastToUInt64( valueLast );
                    break;

                case EQUATION_OPER_USUB:
                    value.ValueUInt64 = CastToUInt64( valuePrev ) - CastToUInt64( valueLast );
                    break;

                case EQUATION_OPER_UDIV:
                {
                    const uint64_t valueLastUint64 = CastToUInt64( valueLast );
                    value.ValueUInt64              = valueLastUint64 != 0ULL
                                     ? CastToUInt64( valuePrev ) / valueLastUint64
                                     : 0ULL;
                    break;
                }

                case EQUATION_OPER_UMUL:
                    value.ValueUInt64 = CastToUInt64( valuePrev ) * CastToUInt64( valueLast );

                    break;

                case EQUATION_OPER_FADD:
                    value.ValueFloat = CastToFloat( valuePrev ) + CastToFloat( valueLast );
                    value.ValueType  = VALUE_TYPE_FLOAT;
                    break;

                case EQUATION_OPER_FSUB:
                    value.ValueFloat = CastToFloat( valuePrev ) - CastToFloat( valueLast );
                    value.ValueType  = VALUE_TYPE_FLOAT;
                    break;

                case EQUATION_OPER_FMUL:
                    value.ValueFloat = CastToFloat( valuePrev ) * CastToFloat( valueLast );
                    value.ValueType  = VALUE_TYPE_FLOAT;
                    break;

                case EQUATION_OPER_FDIV:
                {
                    const float valueLastFloat = CastToFloat( valueLast );
                    value.ValueFloat           = valueLastFloat != 0.0f
                                  ? CastToFloat( valuePrev ) / valueLastFloat
                                  : 0.0f;
                    value.ValueType            = VALUE_TYPE_FLOAT;
                    break;
                }

                case EQUATION_OPER_UGT:
                    value.ValueBool = CastToUInt64( valuePrev ) > CastToUInt64( valueLast );
                    value.ValueType = VALUE_TYPE_BOOL;
                    break;

                case EQUATION_OPER_ULT:
                    value.ValueBool = CastToUInt64( valuePrev ) < CastToUInt64( valueLast );
                    value.ValueType = VALUE_TYPE_BOOL;
                    break;

                case EQUATION_OPER_UGTE:
                    value.ValueBool = CastToUInt64( valuePrev ) >= CastToUInt64( valueLast );
                    value.ValueType = VALUE_TYPE_BOOL;
                    break;

                case EQUATION_OPER_ULTE:
                    value.ValueBool = CastToUInt64( valuePrev ) <= CastToUInt64( valueLast );
                    value.ValueType = VALUE_TYPE_BOOL;
                    break;

                case EQUATION_OPER_FGT:
                    value.ValueBool = CastToFloat( valuePrev ) > CastToFloat( valueLast );
                    value.ValueType = VALUE_TYPE_BOOL;
                    break;

                case EQUATION_OPER_FLT:
                    value.ValueBool = CastToFloat( valuePrev ) < CastToFloat( valueLast );
                    value.ValueType = VALUE_TYPE_BOOL;
                    break;

                case EQUATION_OPER_FGTE:
                    value.ValueBool = CastToFloat( valuePrev ) >= CastToFloat( valueLast );
                    value.ValueType = VALUE_TYPE_BOOL;
                    break;

                case EQUATION_OPER_FLTE:
                    value.ValueBool = CastToFloat( valuePrev ) <= CastToFloat( valueLast );
                    value.ValueType = VALUE_TYPE_BOOL;
                    break;

                case EQUATION_OPER_UMIN:
                    // (std::min) - braces to bypass windows.h min/max errors
                    value.ValueUInt64 = ( std::min )( CastToUInt64( valuePrev ), CastToUInt64( valueLast ) );
                    break;

                case EQUATION_OPER_UMAX:
                    // (std::min) - braces to bypass windows.h min/max errors
                    value.ValueUInt64 = ( std::max )( CastToUInt64( valuePrev ), CastToUInt64( valueLast ) );
                    break;

                case EQUATION_OPER_FMIN:
                    // (std::min) - braces to bypass windows.h min/max errors
                    value.ValueFloat = ( std::min )( CastToFloat( valuePrev ), CastToFloat( valueLast ) );
                    value.ValueType  = VALUE_TYPE_FLOAT;
                    break;

                case EQUATION_OPER_FMAX:
                    // (std::min) - braces to bypass windows.h min/max errors
                    value.ValueFloat = ( std::max )( CastToFloat( valuePrev ), CastToFloat( valueLast ) );
                    value.ValueType  = VALUE_TYPE_FLOAT;
                    break;

                default:
                    MD_ASSERT( false );
                    break;
            }

            return value;
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     EquationStackPush
        //
        // Description:
        //     Pushes an equation to the stack.
        //
        // Input:
        //     std::stack<TTypedValue_1_0>& stack          - equation stack.
        //     TTypedValue_1_0&             value          - value to be pushed.
        //     uint32_t&                    algorithmCheck - algorithm check.
        //
        // Output:
        //     bool - true if an equation was pushed successfully.
        //
        //////////////////////////////////////////////////////////////////////////////
        inline bool EquationStackPush(
            std::stack<TTypedValue_1_0>& stack,
            TTypedValue_1_0&             value,
            uint32_t&                    algorithmCheck )
        {
            stack.push( value );
            algorithmCheck++;
            return ( stack.size() == algorithmCheck );
        }

        //////////////////////////////////////////////////////////////////////////////
        //
        // Class:
        //     CMetricsCalculator
        //
        // Method:
        //     ClearStack
        //
        // Description:
        //     Clears the stack until it is empty.
        //
        // Input:
        //     std::stack<TTypedValue_1_0>& stack - equation stack.
        //
        //////////////////////////////////////////////////////////////////////////////
        inline void ClearStack( std::stack<TTypedValue_1_0>& stack )
        {
            while( !stack.empty() )
            {
                stack.pop();
            }
        }

    private:
        std::stack<TTypedValue_1_0> m_readEquationStack;
        std::stack<TTypedValue_1_0> m_readEquationAndDeltaStack;
        std::stack<TTypedValue_1_0> m_normalizationEquationStack;
        CMetricsDevice&             m_device;
        uint64_t                    m_gpuCoreClocks;
        uint32_t                    m_euCoresCount;
        uint8_t*                    m_savedReport;
        uint32_t                    m_savedReportSize;
        uint64_t                    m_contextIdPrev;
        bool                        m_savedReportPresent;
        TTypedValue_1_0*            m_prevValues;
        uint32_t                    m_prevValuesCount;
    };
} // namespace MetricsDiscoveryInternal
