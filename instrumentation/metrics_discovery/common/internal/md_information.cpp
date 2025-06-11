/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//     File Name:  md_information.cpp

//     Abstract:   C++ Metrics Discovery internal information implementation

#include "md_information.h"
#include "md_adapter.h"
#include "md_equation.h"
#include "md_metrics_device.h"

#include "md_utils.h"

namespace MetricsDiscoveryInternal
{

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     CInformation constructor
    //
    // Description:
    //     Constructor.
    //
    // Input:
    //     CMetricsDevice&  device           -
    //     uint32_t         id               -
    //     const char*      name             -
    //     const char*      shortName        -
    //     const char*      longName         -
    //     const char*      group            -
    //     uint32_t         apiMask          -
    //     TInformationType informationType  -
    //     const char       informationUnits -
    //
    //////////////////////////////////////////////////////////////////////////////
    CInformation::CInformation( CMetricsDevice& device, uint32_t id, const char* name, const char* shortName, const char* longName, const char* group, uint32_t apiMask, TInformationType informationType, const char* informationUnits )
        : m_params{}
        , m_id( id ) // original, equal to filtered on creation
        , m_ioReadEquation( nullptr )
        , m_availabilityEquation( nullptr )
        , m_queryReadEquation( nullptr )
        , m_device( device )
    {
        const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

        m_params.IdInSet    = id; // filtered, equal to original on creation
        m_params.SymbolName = GetCopiedCString( name, adapterId );
        m_params.ShortName  = GetCopiedCString( shortName, adapterId );
        m_params.LongName   = GetCopiedCString( longName, adapterId );
        m_params.GroupName  = GetCopiedCString( group, adapterId );
        m_params.ApiMask    = apiMask;
        m_params.InfoType   = informationType;
        m_params.InfoUnits  = GetCopiedCString( informationUnits, adapterId );

        m_params.OverflowFunction.FunctionType = DELTA_FUNCTION_NULL;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     CInformation copy constructor
    //
    // Description:
    //     Copy constructor.
    //
    //////////////////////////////////////////////////////////////////////////////
    CInformation::CInformation( const CInformation& other )
        : m_params{}
        , m_id( other.m_id ) // initial id before filterings
        , m_device( other.m_device )
    {
        const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

        m_params.IdInSet    = other.m_params.IdInSet; // id after filterings
        m_params.SymbolName = GetCopiedCString( other.m_params.SymbolName, adapterId );
        m_params.ShortName  = GetCopiedCString( other.m_params.ShortName, adapterId );
        m_params.GroupName  = GetCopiedCString( other.m_params.GroupName, adapterId );
        m_params.LongName   = GetCopiedCString( other.m_params.LongName, adapterId );
        m_params.ApiMask    = other.m_params.ApiMask;
        m_params.InfoType   = other.m_params.InfoType;
        m_params.InfoUnits  = GetCopiedCString( other.m_params.InfoUnits, adapterId );

        m_params.OverflowFunction = other.m_params.OverflowFunction;

        m_availabilityEquation = other.m_availabilityEquation ? new( std::nothrow ) CEquation( *other.m_availabilityEquation ) : nullptr;
        m_ioReadEquation       = other.m_ioReadEquation ? new( std::nothrow ) CEquation( *other.m_ioReadEquation ) : nullptr;
        m_queryReadEquation    = other.m_queryReadEquation ? new( std::nothrow ) CEquation( *other.m_queryReadEquation ) : nullptr;

        m_params.IoReadEquation    = static_cast<IEquation_1_0*>( m_ioReadEquation );
        m_params.QueryReadEquation = static_cast<IEquation_1_0*>( m_queryReadEquation );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     ~CInformation destructor
    //
    // Description:
    //     Deallocates memory.
    //
    //////////////////////////////////////////////////////////////////////////////
    CInformation::~CInformation()
    {
        MD_SAFE_DELETE_ARRAY( m_params.SymbolName );
        MD_SAFE_DELETE_ARRAY( m_params.ShortName );
        MD_SAFE_DELETE_ARRAY( m_params.LongName );
        MD_SAFE_DELETE_ARRAY( m_params.GroupName );
        MD_SAFE_DELETE_ARRAY( m_params.InfoUnits );
        MD_SAFE_DELETE( m_availabilityEquation );
        MD_SAFE_DELETE( m_ioReadEquation );
        MD_SAFE_DELETE( m_queryReadEquation );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     GetParams
    //
    // Description:
    //     Returns the information params.
    //
    // Output:
    //     TInformationParams_1_0* - pointer to the information params
    //
    //////////////////////////////////////////////////////////////////////////////
    TInformationParams_1_0* CInformation::GetParams( void )
    {
        return &m_params;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     SetSnapshotReportReadEquation
    //
    // Description:
    //     Sets the snapshot (IO) read equation in the information.
    //
    // Input:
    //     const char * equationString - equation string, could be empty
    //
    // Output:
    //     TCompletionCode             - result of the operation
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CInformation::SetSnapshotReportReadEquation( const char* equationString )
    {
        const auto ret          = SetEquation( m_device, m_ioReadEquation, equationString );
        m_params.IoReadEquation = static_cast<IEquation_1_0*>( m_ioReadEquation );

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     SetDeltaReportReadEquation
    //
    // Description:
    //     Sets the delta (query) read equation in the information.
    //
    // Input:
    //     const char * equationString - equation string, could be empty
    //
    // Output:
    //     TCompletionCode             - result of the operation
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CInformation::SetDeltaReportReadEquation( const char* equationString )
    {
        const auto ret             = SetEquation( m_device, m_queryReadEquation, equationString );
        m_params.QueryReadEquation = static_cast<IEquation_1_0*>( m_queryReadEquation );

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     SetAvailabilityEquation
    //
    // Description:
    //     Sets the availability equation in the information. It's used to determine if
    //     the information is available on the current platform.
    //
    // Input:
    //     const char * equationString - equation string, could be empty
    //
    // Output:
    //     TCompletionCode             - result of the operation
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CInformation::SetAvailabilityEquation( const char* equationString )
    {
        return SetEquation( m_device, m_availabilityEquation, equationString );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     IsAvailabilityEquationTrue
    //
    // Description:
    //     Solves the given information availability equation.
    //
    // Output:
    //     bool - result of solving availability equation.
    //
    //////////////////////////////////////////////////////////////////////////////
    bool CInformation::IsAvailabilityEquationTrue()
    {
        return !m_availabilityEquation || m_availabilityEquation->SolveBooleanEquation();
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     IsAggregatable
    //
    // Description:
    //     Returns true if information can be aggregated, false otherwise.
    //
    // Output:
    //     bool - true if information can be aggregated, false otherwise.
    //
    //////////////////////////////////////////////////////////////////////////////
    bool CInformation::IsAggregatable() const
    {
        return !( m_params.ApiMask & API_TYPE_IOSTREAM ) || std::string_view( m_params.SymbolName ) == "QueryBeginTime";
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CMetric
    //
    // Method:
    //     SetOverflowFunction
    //
    // Description:
    //     Sets the overflow delta equation in the information.
    //     It's an equation to calculate whether the overflow occurs.
    //
    // Input:
    //     const char* equationString - equation string, could be empty
    //
    // Output:
    //     TCompletionCode            - result of the operation
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CInformation::SetOverflowFunction( const char* equationString )
    {
        return SetDeltaFunction( equationString, &m_params.OverflowFunction, m_device.GetAdapter().GetAdapterId() );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CMetric
    //
    // Method:
    //     SetOverflowFunction
    //
    // Description:
    //     Sets the overflow delta equation in the information.
    //     It's an equation to calculate whether the overflow occurs.
    //
    // Input:
    //     const char* equationString - equation string, could be empty
    //
    // Output:
    //     TCompletionCode            - result of the operation
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CInformation::SetOverflowFunction( TDeltaFunction_1_0 overflowFunction )
    {
        m_params.OverflowFunction = overflowFunction;

        return CC_OK;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     WriteCInformationToBuffer
    //
    // Description:
    //     Write the information object to buffer.
    //
    // Input:
    //     uint8_t*  buffer       - pointer to a buffer
    //     uint32_t& bufferSize   - size of the buffer
    //     uint32_t& bufferOffset - the current offset of the buffer
    //
    // Output:
    //     TCompletionCode        - result of the operation
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CInformation::WriteCInformationToBuffer( uint8_t* buffer, uint32_t& bufferSize, uint32_t& bufferOffset )
    {
        const uint32_t  adapterId = m_device.GetAdapter().GetAdapterId();
        TCompletionCode result    = CC_OK;

        // m_params
        result = WriteCStringToBuffer( m_params.SymbolName, buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        result = WriteCStringToBuffer( m_params.ShortName, buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        result = WriteCStringToBuffer( m_params.GroupName, buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        result = WriteCStringToBuffer( m_params.LongName, buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        result = WriteDataToBuffer( &m_params.ApiMask, sizeof( m_params.ApiMask ), buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        result = WriteDataToBuffer( &m_params.InfoType, sizeof( m_params.InfoType ), buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        result = WriteCStringToBuffer( m_params.InfoUnits, buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        // Availability equation
        result = WriteEquationToBuffer( m_availabilityEquation, buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        // OverflowFunction
        result = WriteDataToBuffer( &m_params.OverflowFunction.FunctionType, sizeof( m_params.OverflowFunction.FunctionType ), buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        result = WriteDataToBuffer( &m_params.OverflowFunction.BitsCount, sizeof( m_params.OverflowFunction.BitsCount ), buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        // Equations
        result = WriteEquationToBuffer( m_ioReadEquation, buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        result = WriteEquationToBuffer( m_queryReadEquation, buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        return CC_OK;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     SetInformationValue
    //
    // Description:
    //     Sets the value for the information as a given equation.
    //
    // Input:
    //     const uint32_t      value        - information value
    //     const TEquationType equationType - equation to be set
    //
    // Output:
    //     TCompletionCode                  - result of operation
    //
    //////////////////////////////////////////////////////////////////////////////
    TCompletionCode CInformation::SetInformationValue( const uint32_t value, const TEquationType equationType )
    {
        CEquation* equation = ( equationType == EQUATION_IO_READ )
            ? m_ioReadEquation
            : ( equationType == EQUATION_QUERY_READ )
            ? m_queryReadEquation
            : nullptr;

        if( equation != nullptr &&
            equation->GetEquationElementsCount() == 1 &&
            equation->GetEquationElement( 0 )->Type == EQUATION_ELEM_IMM_UINT64 )
        {
            equation->GetEquationElement( 0 )->ImmediateUInt64 = value;
        }
        else
        {
            return CC_ERROR_INVALID_PARAMETER;
        }

        return CC_OK;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     SetIdInSetParam
    //
    // Description:
    //     Updates IdInSet parameter in the information element.
    //
    // Input:
    //     uint32_t id - id in current information set
    //
    // Output:
    //     void        - return values is absent
    //
    //////////////////////////////////////////////////////////////////////////////
    void CInformation::SetIdInSetParam( uint32_t id )
    {
        m_params.IdInSet = id;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CInformation
    //
    // Method:
    //     GetId
    //
    // Description:
    //     Returns information position in metric set.
    //
    // Output:
    //     uint32_t - id
    //
    //////////////////////////////////////////////////////////////////////////////
    uint32_t CInformation::GetId() const
    {
        return m_id;
    }

} // namespace MetricsDiscoveryInternal
