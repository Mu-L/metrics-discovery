/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//     File Name:  md_metrics_device.h

//     Abstract:   C++ Metrics Discovery internal metrics device header

#pragma once

#include "md_symbol_set.h"

#include <vector>

#define MD_METRICS_FILE_KEY     "CUSTOM_METRICS_FILE\n"
#define MD_METRICS_FILE_KEY_2_0 "CUSTOM_METRICS_FILE_2_0\n"
#define MD_METRICS_FILE_KEY_3_0 "CUSTOM_METRICS_FILE_3_0\n"

#define OBTAIN_ADAPTER_ID( device ) device ? device->GetAdapter().GetAdapterId() : IU_ADAPTER_ID_UNKNOWN

using namespace MetricsDiscovery;

namespace MetricsDiscoveryInternal
{
    ///////////////////////////////////////////////////////////////////////////////
    // Forward declarations:                                                     //
    ///////////////////////////////////////////////////////////////////////////////
    class CAdapter;
    class CConcurrentGroup;
    class CDriverInterface;
    class CMetricSet;

    ///////////////////////////////////////////////////////////////////////////////
    // Custom metric file version:                                               //
    ///////////////////////////////////////////////////////////////////////////////
    typedef enum ECustomMetricsFileVersion
    {
        CUSTOM_METRICS_FILE_VERSION_0 = 0,
        CUSTOM_METRICS_FILE_VERSION_1 = 1,
        CUSTOM_METRICS_FILE_VERSION_2 = 2,
        CUSTOM_METRICS_FILE_VERSION_3 = 3,
        CUSTOM_METRICS_FILE_CURRENT   = CUSTOM_METRICS_FILE_VERSION_3,
    } TCustomMetricsFileVersion;

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CMetricsDevice
    //
    // Description:
    //     GPU metrics root object. Stores all the concurrent groups and global symbols.
    //
    //////////////////////////////////////////////////////////////////////////////
    class CMetricsDevice : public IMetricsDeviceLatest
    {
    public:
        virtual IConcurrentGroupLatest* GetConcurrentGroup( uint32_t index );

        // API 1.10:
        virtual TCompletionCode GetGpuCpuTimestamps( uint64_t* gpuTimestampNs, uint64_t* cpuTimestampNs, uint32_t* cpuId, uint64_t* correlationIndicatorNs );

        // API 1.2:
        virtual IOverride_1_2* GetOverride( uint32_t index );
        virtual IOverride_1_2* GetOverrideByName( const char* symbolName );

        // API 1.0:
        virtual TMetricsDeviceParams_1_2* GetParams( void );
        virtual TGlobalSymbolLatest*      GetGlobalSymbol( uint32_t index );
        virtual TTypedValueLatest*        GetGlobalSymbolValueByName( const char* name );
        virtual TCompletionCode           GetLastError();
        virtual TCompletionCode           GetGpuCpuTimestamps( uint64_t* gpuTimestampNs, uint64_t* cpuTimestampNs, uint32_t* cpuId );

    public:
        // Constructor & Destructor:
        CMetricsDevice( CAdapter& adapter, CDriverInterface& driverInterface, const uint32_t subDeviceIndex = 0, const bool isOffline = false );
        virtual ~CMetricsDevice();

        CMetricsDevice( const CMetricsDevice& )            = delete; // Delete copy-constructor
        CMetricsDevice& operator=( const CMetricsDevice& ) = delete; // Delete assignment operator

        // Non-API:
        CConcurrentGroup* AddConcurrentGroup( const char* symbolicName, const char* shortName, const uint32_t measurementTypeMask, TByteArrayLatest* platformMask, bool& isSupported );

        TCompletionCode AddOverrides();
        bool            IsPlatformTypeOf( TByteArrayLatest* platformMask, uint32_t gtMask = GT_TYPE_ALL );

        TCompletionCode   SaveToFile( const char* fileName, const uint32_t minMajorApiVersion, const uint32_t minMinorApiVersion );
        TCompletionCode   WriteToBuffer( uint8_t* buffer, uint32_t& bufferSize, IMetricSet_1_13** metricSets, uint32_t metricSetCount, const uint32_t minMajorApiVersion, const uint32_t minMinorApiVersion );
        TCompletionCode   OpenFromFile( const char* fileName );
        TCompletionCode   OpenOfflineFromBuffer( uint8_t* buffer, uint32_t bufferSize );
        TQueryMode        GetQueryMode() const;
        CConcurrentGroup* GetConcurrentGroupByName( const char* symbolicName );
        CDriverInterface& GetDriverInterface();
        CAdapter&         GetAdapter();
        CSymbolSet&       GetSymbolSet();
        uint32_t          GetPlatformIndex();
        bool              IsOpenedFromFile();
        uint64_t          ConvertGpuTimestampToNs( const uint64_t gpuTimestampTicks, const uint64_t gpuTimestampFrequency );

        // Reference counter.
        uint32_t& GetReferenceCounter();

        // Sub devices.
        uint32_t GetSubDeviceIndex();

        GTDI_OA_BUFFER_MASK GetOaBufferMask();

        // Performance stream.
        int32_t               GetStreamId();
        int32_t               GetStreamConfigId();
        void                  SetStreamId( const int32_t id );
        void                  SetStreamConfigId( const int32_t id );
        std::vector<uint8_t>& GetStreamBuffer();

    private:
        // Methods to read from buffer must be used in correct order
        TCompletionCode ReadGlobalSymbolsFromBuffer( uint8_t*& bufferPtr, const uint8_t* bufferBeginOffset, const uint32_t bufferSize, const uint32_t bufferVersion );
        TCompletionCode ReadConcurrentGroupsFromBuffer( uint8_t*& bufferPtr, const uint8_t* bufferBeginOffset, const uint32_t bufferSize, TApiVersion_1_0* apiVersion, const uint32_t bufferVersion );
        TCompletionCode ReadMetricSetsFromBuffer( uint8_t*& bufferPtr, const uint8_t* bufferBeginOffset, const uint32_t bufferSize, CConcurrentGroup* group, TApiVersion_1_0* apiVersion, const uint32_t bufferVersion );
        TCompletionCode ReadMetricsFromBuffer( uint8_t*& bufferPtr, const uint8_t* bufferBeginOffset, const uint32_t bufferSize, CMetricSet* set, const bool isSetNew );
        TCompletionCode ReadInformationFromBuffer( uint8_t*& bufferPtr, const uint8_t* bufferBeginOffset, const uint32_t bufferSize, CMetricSet* set );
        TCompletionCode ReadRegistersFromBuffer( uint8_t*& bufferPtr, const uint8_t* bufferBeginOffset, const uint32_t bufferSize, CMetricSet* set );

        IOverrideLatest* AddOverride( TOverrideType overrideType );
        bool             IsMetricsFileInPlainTextFormat( FILE* metricFile, uint32_t& fileVersion );

    private:
        // Variables:
        TMetricsDeviceParamsLatest     m_params;
        std::vector<CConcurrentGroup*> m_groupsVector;
        std::vector<IOverrideLatest*>  m_overridesVector;
        CAdapter&                      m_adapter;
        CDriverInterface&              m_driverInterface;
        CSymbolSet                     m_symbolSet;

        // Stream:
        int32_t              m_streamId;
        int32_t              m_streamConfigId;
        std::vector<uint8_t> m_streamBuffer;

        // Sub device:
        uint32_t m_subDeviceIndex;

        uint32_t m_platformIndex;
        TGTType  m_gtType;
        bool     m_isOpenedFromFile;
        bool     m_isOffline;
        uint32_t m_referenceCounter;

        GTDI_OA_BUFFER_MASK m_oaBufferMask;

        TQueryMode       m_queryModeRequested;
        const TQueryMode m_queryModeDefault;
    };
} // namespace MetricsDiscoveryInternal
