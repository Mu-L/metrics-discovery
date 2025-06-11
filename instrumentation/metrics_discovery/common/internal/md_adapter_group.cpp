/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//     File Name:  md_adapter_group.cpp

//     Abstract:   C++ Metrics Discovery internal adapter group implementation

#include "md_adapter_group.h"
#include "md_adapter.h"
#include "md_metrics_device.h"

#include "md_driver_ifc.h"
#include "md_driver_ifc_offline.h"
#include "md_utils.h"

namespace MetricsDiscoveryInternal
{
    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     CAdapterGroup
    //
    // Description:
    //     Constructor.
    //
    //////////////////////////////////////////////////////////////////////////////
    CAdapterGroup::CAdapterGroup()
        : m_params{}
        , m_defaultAdapter( nullptr )
        , m_adapterVector()
        , m_offlineAdapter( nullptr )
        , m_offlineDriverInterface( nullptr )
        , m_offlineDevicesVector()
    {
        m_params.Version.MajorNumber = MD_API_MAJOR_NUMBER_CURRENT;
        m_params.Version.MinorNumber = MD_API_MINOR_NUMBER_CURRENT;
        m_params.Version.BuildNumber = MD_API_BUILD_NUMBER_CURRENT;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     ~CAdapterGroup
    //
    // Description:
    //     Deallocates memory.
    //
    //////////////////////////////////////////////////////////////////////////////
    CAdapterGroup::~CAdapterGroup()
    {
        CleanupAdapters();

        ClearVector( m_offlineDevicesVector );
        MD_SAFE_DELETE( m_offlineDriverInterface );
        MD_SAFE_DELETE( m_offlineAdapter );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     GetParams
    //
    // Description:
    //     Returns adapter group params.
    //
    // Output:
    //     const TAdapterGroupParams_1_6* - adapter group params
    //
    //////////////////////////////////////////////////////////////////////////////
    const TAdapterGroupParams_1_6* CAdapterGroup::GetParams() const
    {
        return &m_params;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     GetAdapter
    //
    // Description:
    //    Returns chosen adapter or null if index doesn't exist.
    //
    // Input:
    //     uint32_t index - index of a chosen adapter
    //
    // Output:
    //     IAdapterLatest* - chosen adapter or null
    //
    //////////////////////////////////////////////////////////////////////////////
    IAdapterLatest* CAdapterGroup::GetAdapter( uint32_t index )
    {
        if( index < m_adapterVector.size() )
        {
            return m_adapterVector[index];
        }

        return nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     Close
    //
    // Description:
    //     Decreases adapter group reference counter and closes it (frees up recourses)
    //     if the counter reaches 0.
    //
    // Output:
    //     TCompletionCode - CC_OK or CC_STILL_INITIALIZED means success
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CAdapterGroup::Close()
    {
        MD_LOG_ENTER();
        MD_ASSERT( this == m_adapterGroup );

        TCompletionCode retVal = GetOpenCloseSemaphore();
        if( retVal != CC_OK )
        {
            MD_LOG( LOG_ERROR, "Get semaphore failed" );
            MD_LOG_EXIT();
            return retVal;
        }

        if( m_agRefCounter > 1 )
        {
            m_agRefCounter--;
            retVal = CC_STILL_INITIALIZED;
        }
        else if( m_agRefCounter == 1 )
        {
            m_agRefCounter = 0;
            retVal         = CC_OK;
        }
        else
        {
            retVal = CC_ERROR_GENERAL;
        }

        ReleaseOpenCloseSemaphore();

        if( m_agRefCounter == 0 )
        {
            // Important: 'this' (current object) is deleted here
            MD_SAFE_DELETE( m_adapterGroup );
        }

        MD_LOG_EXIT();
        return retVal;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     Open
    //
    // Description:
    //     Opens main MDAPI root object - adapter group or retrieves an instance
    //     opened before. Only one instance of adapter group may be created, all
    //     Open() calls are reference counted.
    //
    // Input:
    //     CAdapterGroup** adapterGroup - [out] created / retrieved adapter group
    //
    // Output:
    //     TCompletionCode              - CC_OK or CC_ALREADY_INITIALIZED means success
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CAdapterGroup::Open( CAdapterGroup** adapterGroup )
    {
        MD_LOG_ENTER();
        MD_CHECK_PTR_RET( adapterGroup, CC_ERROR_INVALID_PARAMETER );

        TCompletionCode retVal = GetOpenCloseSemaphore();
        if( retVal != CC_OK )
        {
            MD_LOG( LOG_ERROR, "Get semaphore failed" );
            MD_LOG_EXIT();
            return retVal;
        }

        if( m_adapterGroup )
        {
            *adapterGroup = m_adapterGroup;
            retVal        = CC_ALREADY_INITIALIZED;
            m_agRefCounter++;
        }
        else
        {
            // Read global debug log settings
            CDriverInterface::ReadDebugLogSettings();

            retVal = CreateAdapterGroup( adapterGroup );
            if( retVal == CC_OK )
            {
                m_agRefCounter++;
            }
        }

        ReleaseOpenCloseSemaphore();

        MD_LOG_EXIT();
        return retVal;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     OpenOfflineMetricsDeviceFromBuffer
    //
    // Description:
    //     Opens offline metrics device object.
    //     Multiple instances of offline metric devices may be created at once.
    //
    // Input:
    //     uint8_t*              buffer        - a buffer that an offline device is created from
    //     uint32_t              bufferSize    - the size of a buffer
    //     IMetricsDevice_1_13** metricsDevice - [out] created / retrieved metrics device
    //
    // Output:
    //     TCompletionCode                     - CC_OK or CC_ALREADY_INITIALIZED means success
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CAdapterGroup::OpenOfflineMetricsDeviceFromBuffer( uint8_t* buffer, uint32_t bufferSize, IMetricsDevice_1_13** metricsDevice )
    {
        MD_CHECK_PTR_RET( buffer, CC_ERROR_INVALID_PARAMETER );
        MD_CHECK_PTR_RET( metricsDevice, CC_ERROR_INVALID_PARAMETER );

        CMetricsDevice* offlineDevice = nullptr;
        TCompletionCode result        = CC_OK;

        if( !m_offlineAdapter )
        {
            m_offlineAdapter = new( std::nothrow ) CAdapter( *this );
            MD_CHECK_PTR( m_offlineAdapter );
        }

        if( !m_offlineDriverInterface )
        {
            m_offlineDriverInterface = new( std::nothrow ) CDriverInterfaceOffline();
            MD_CHECK_PTR( m_offlineDriverInterface );
        }

        offlineDevice = new( std::nothrow ) CMetricsDevice( *m_offlineAdapter, *m_offlineDriverInterface, 0, true );
        MD_CHECK_PTR( offlineDevice );

        result = offlineDevice->OpenOfflineFromBuffer( buffer, bufferSize );
        MD_CHECK_CC( result );

        *metricsDevice = offlineDevice;

        m_offlineDevicesVector.push_back( offlineDevice );

        return result;

    exception:
        MD_SAFE_DELETE( offlineDevice );

        if( m_offlineDevicesVector.size() == 0 )
        {
            MD_SAFE_DELETE( m_offlineDriverInterface );
            MD_SAFE_DELETE( m_offlineAdapter );
        }

        return CC_ERROR_NO_MEMORY;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     CloseOfflineMetricsDevice
    //
    // Description:
    //     Close offline metrics device object and free resources.
    //
    // Input:
    //     IMetricsDevice_1_13* metricsDevice - a pointer to offline metrics device to close
    //
    // Output:
    //     TCompletionCode                    - CC_OK means success
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CAdapterGroup::CloseOfflineMetricsDevice( IMetricsDevice_1_13* metricsDevice )
    {
        auto deviceIterator = std::find( m_offlineDevicesVector.begin(), m_offlineDevicesVector.end(), metricsDevice );

        if( deviceIterator == m_offlineDevicesVector.end() )
        {
            return CC_ERROR_INVALID_PARAMETER;
        }

        auto offlineDevice = static_cast<CMetricsDevice*>( *deviceIterator );

        MD_SAFE_DELETE( offlineDevice );
        m_offlineDevicesVector.erase( deviceIterator );

        if( m_offlineDevicesVector.size() == 0 )
        {
            MD_SAFE_DELETE( m_offlineDriverInterface );
            MD_SAFE_DELETE( m_offlineAdapter );
        }

        return CC_OK;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     SaveMetricsDeviceToBuffer
    //
    // Description:
    //     Saves metrics device to buffer. Then the buffer can be used for offline calculation.
    //
    // Input:
    //     IMetricsDevice_1_13* metricsDevice      - a buffer that an offline device is created from
    //     IMetricSet_1_13**    metricSets         - an array of metric sets that will be written to the buffer
    //     uint32_t             metricSetCount     - a number of metric sets in the array
    //     uint8_t*             buffer             - a buffer that an offline device is created from
    //     uint32_t             bufferSize         - the size of a buffer
    //     const uint32_t       minMajorApiVersion - required MDAPI major version to open the buffer
    //     const uint32_t       minMinorApiVersion - required MDAPI minor version to open the buffer
    //
    // Output:
    //     TCompletionCode                         - CC_OK means success
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CAdapterGroup::SaveMetricsDeviceToBuffer( IMetricsDevice_1_13* metricsDevice, IMetricSet_1_13** metricSets, uint32_t metricSetCount, uint8_t* buffer, uint32_t* bufferSize, const uint32_t minMajorApiVersion, const uint32_t minMinorApiVersion )
    {
        MD_CHECK_PTR_RET( metricsDevice, CC_ERROR_INVALID_PARAMETER );
        MD_CHECK_PTR_RET( metricSets, CC_ERROR_INVALID_PARAMETER );
        MD_CHECK_PTR_RET( bufferSize, CC_ERROR_INVALID_PARAMETER );

        CMetricsDevice* device = static_cast<CMetricsDevice*>( metricsDevice );

        return device->WriteToBuffer( buffer, *bufferSize, metricSets, metricSetCount, minMajorApiVersion, minMinorApiVersion );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     IsOpened
    //
    // Description:
    //     Checks whether adapter group was opened (created) before.
    //
    // Output:
    //     bool - true if adapter group instance exists (was created before)
    //
    //////////////////////////////////////////////////////////////////////////////
    bool CAdapterGroup::IsOpened()
    {
        return m_adapterGroup != nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     Get
    //
    // Description:
    //     Returns static adapter group instance.
    //
    // Output:
    //     CAdapterGroup* - adapter group instance, may be null if it doesn't exists
    //
    //////////////////////////////////////////////////////////////////////////////
    CAdapterGroup* CAdapterGroup::Get()
    {
        return m_adapterGroup;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     GetOpenCloseSemaphore
    //
    // Description:
    //     Acquires semaphore used during adapter group open / close operations.
    //
    // Output:
    //     TCompletionCode - CC_OK means success
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CAdapterGroup::GetOpenCloseSemaphore()
    {
        return GetNamedSemaphore( "AdOpenClose", &m_openCloseSemaphore, IU_ADAPTER_ID_UNKNOWN );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     ReleaseOpenCloseSemaphore
    //
    // Description:
    //     Releases semaphore used during adapter group open / close operations.
    //
    // Output:
    //     TCompletionCode - CC_OK means success
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CAdapterGroup::ReleaseOpenCloseSemaphore()
    {
        return ReleaseNamedSemaphore( &m_openCloseSemaphore, IU_ADAPTER_ID_UNKNOWN );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     CreateAdapterGroup
    //
    // Description:
    //     Creates adapter group along with the whole adapter tree, including
    //     adapter enumeration.
    //
    // Input:
    //     CAdapterGroup** adapterGroup - [optional] created adapter group
    //
    // Output:
    //     TCompletionCode              - CC_OK if success
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CAdapterGroup::CreateAdapterGroup( CAdapterGroup** adapterGroup )
    {
        MD_ASSERT( m_adapterGroup == nullptr );

        m_adapterGroup = new( std::nothrow ) CAdapterGroup();
        MD_CHECK_PTR_RET( m_adapterGroup, CC_ERROR_NO_MEMORY );

        TCompletionCode ret = m_adapterGroup->CreateAdapterTree();
        if( ret != CC_OK )
        {
            MD_SAFE_DELETE( m_adapterGroup );
            return ret;
        }

        if( adapterGroup )
        {
            *adapterGroup = m_adapterGroup;
        }

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     GetDefaultAdapter
    //
    // Description:
    //     Returns default adapter chosen during initialization.
    //
    // Output:
    //     CAdapter* - default adapter, may be null if no adapters are available
    //
    //////////////////////////////////////////////////////////////////////////////
    CAdapter* CAdapterGroup::GetDefaultAdapter()
    {
        MD_LOG( LOG_DEBUG, "Returned default adapter: %p", m_defaultAdapter );
        return m_defaultAdapter;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     CreateAdapterTree
    //
    // Description:
    //     Creates the whole adapter tree. Includes available adapter discovery,
    //     creating adapter objects and filling their data.
    //
    // Output:
    //     TCompletionCode - CC_OK means success
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CAdapterGroup::CreateAdapterTree()
    {
        MD_LOG_ENTER();

        std::vector<TAdapterData> availableAdapters;

        // 1. Get adapter information from OS
        auto ret = CDriverInterface::GetAvailableAdapters( availableAdapters );
        MD_CHECK_CC_RET( ret );

        // 2. Create adapter objects
        for( const auto& adapterData : availableAdapters )
        {
            ret = AddAdapter( adapterData );
            if( ret != CC_OK )
            {
                MD_LOG( LOG_ERROR, "Error: failed to add adapter %s", adapterData.Params.ShortName );
                CleanupAdapters();
                break;
            }
        }
        MD_ASSERT( m_params.AdapterCount == m_adapterVector.size() );

        // 3. Choose default adapter
        if( ret == CC_OK )
        {
            m_defaultAdapter = ChooseDefaultAdapter();
            MD_LOG( LOG_INFO, "Default adapter: %s", m_defaultAdapter ? m_defaultAdapter->GetParams()->ShortName : "None" );
        }

        MD_LOG_EXIT();
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     AddAdapter
    //
    // Description:
    //     Creates a single adapter and adds it to the adapter vector. Updates
    //     adapter count in params respectively.
    //     Adapter object becomes owner of the adapter handle (CAdapterHandle object) and
    //     params memory (strings).
    //
    // Input:
    //     const TAdapterData& adapterData - input adapter information
    //
    // Output:
    //     TCompletionCode                 - CC_OK means success
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CAdapterGroup::AddAdapter( const TAdapterData& adapterData )
    {
        MD_CHECK_PTR_RET( adapterData.Handle, CC_ERROR_INVALID_PARAMETER );

        auto adapter = new( std::nothrow ) CAdapter( *this, adapterData.Params, *adapterData.Handle );
        MD_CHECK_PTR_RET( adapter, CC_ERROR_NO_MEMORY );

        m_adapterVector.push_back( adapter );
        m_params.AdapterCount = static_cast<uint32_t>( m_adapterVector.size() );

        const TAdapterParamsLatest* adapterParams = adapter->GetParams();
        const uint32_t              adapterId     = adapter->GetAdapterId();

        MD_LOG_A( adapterId, LOG_INFO, "Adapter %s - added", adapterParams->ShortName );
        MD_LOG_A( adapterId, LOG_INFO, "Platform ID: %u", adapterParams->Platform );
        MD_LOG_A( adapterId, LOG_INFO, "Device ID: %x", adapterParams->DeviceId );
        MD_LOG_A( adapterId, LOG_INFO, "Adapter ID: %u", adapterId );

        return CC_OK;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     CleanupAdapters
    //
    // Description:
    //     Cleans all stored adapter. Their handles are closed in their destructors.
    //
    //////////////////////////////////////////////////////////////////////////////
    void CAdapterGroup::CleanupAdapters()
    {
        m_defaultAdapter      = nullptr;
        m_params.AdapterCount = 0;

        ClearVector( m_adapterVector );

        CDriverInterface::ReleaseResources();
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CAdapterGroup
    //
    // Method:
    //     ChooseDefaultAdapter
    //
    // Description:
    //     Chooses default adapter for usage with legacy OpenMetricsDevice API.
    //     Currently default is first external GPU.
    //
    // Output:
    //     CAdapter* - default adapter or nullptr if adapter count if 0
    //
    //////////////////////////////////////////////////////////////////////////////
    CAdapter* CAdapterGroup::ChooseDefaultAdapter()
    {
        if( !m_adapterVector.size() )
        {
            return nullptr;
        }

        // 1. Initialize to first adapter
        CAdapter* defaultAdapter = m_adapterVector.front();

        // 2. Change to discrete GPU if exists
        for( auto& adapter : m_adapterVector )
        {
            if( adapter && adapter->GetParams()->Type == ADAPTER_TYPE_DISCRETE )
            {
                defaultAdapter = adapter;
                break;
            }
        }

        return defaultAdapter;
    }
} // namespace MetricsDiscoveryInternal
