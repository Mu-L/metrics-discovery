/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//     File Name:  md_equation.cpp

//     Abstract:   C++ Metrics Discovery internal equation implementation

#include "md_equation.h"
#include "md_adapter.h"
#include "md_metrics_device.h"

#include "md_utils.h"

#include <cstring>

namespace MetricsDiscoveryInternal
{

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquationElementInternal
    //
    // Method:
    //     CEquationElementInternal constructor
    //
    // Description:
    //     Constructor.
    //
    //////////////////////////////////////////////////////////////////////////////
    CEquationElementInternal::CEquationElementInternal()
        : MetricIndexInternal( -1 )
    {
        Type            = EQUATION_ELEM_LAST_1_0;
        ImmediateUInt64 = 0ULL;
        ImmediateFloat  = 0.0f;
        Mask            = { 0, nullptr };
        Operation       = EQUATION_OPER_LAST_1_0;
        ReadParams      = { 0, 0, 0, 0 };
        SymbolName      = new( std::nothrow ) char[1](); // SymbolName cannot be null, must be an empty string.
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquationElementInternal
    //
    // Method:
    //     CEquationElementInternal destructor
    //
    // Description:
    //     Destructor.
    //
    //////////////////////////////////////////////////////////////////////////////
    CEquationElementInternal::~CEquationElementInternal()
    {
        if( Type == EQUATION_ELEM_MASK )
        {
            DeleteByteArray( Mask, IU_ADAPTER_ID_UNKNOWN );
        }

        MD_SAFE_DELETE_ARRAY( SymbolName );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquationElementInternal
    //
    // Method:
    //     CEquationElementInternal copy constructor
    //
    // Description:
    //     CEquationElementInternal copy constructor
    //
    //////////////////////////////////////////////////////////////////////////////
    CEquationElementInternal::CEquationElementInternal( const CEquationElementInternal& element )
    {
        if( this != &element )
        {
            SetMembers( element );
            CopyMembers( element );
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquationElementInternal
    //
    // Method:
    //     CEquationElementInternal move constructor
    //
    // Description:
    //     CEquationElementInternal move constructor
    //
    //////////////////////////////////////////////////////////////////////////////
    CEquationElementInternal::CEquationElementInternal( CEquationElementInternal&& element )
    {
        if( this != &element )
        {
            SetMembers( element );
            MoveMembers( element );
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquationElementInternal
    //
    // Method:
    //     operator=
    //
    // Description:
    //     Assignment operator. Copies whole memory.
    //
    //////////////////////////////////////////////////////////////////////////////
    CEquationElementInternal& CEquationElementInternal::operator=( const CEquationElementInternal& element )
    {
        if( this != &element )
        {
            SetMembers( element );
            CopyMembers( element );
        }

        return *this;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquationElementInternal
    //
    // Method:
    //     operator=
    //
    // Description:
    //     Assignment operator. Moves whole memory.
    //
    //////////////////////////////////////////////////////////////////////////////
    CEquationElementInternal& CEquationElementInternal::operator=( CEquationElementInternal&& element )
    {
        if( this != &element )
        {
            SetMembers( element );
            MoveMembers( element );
        }

        return *this;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquationElementInternal
    //
    // Method:
    //     SetMembers
    //
    // Description:
    //     Sets all non-dynamically allocated members.
    //
    //////////////////////////////////////////////////////////////////////////////
    void CEquationElementInternal::SetMembers( const CEquationElementInternal& element )
    {
        Type                = element.Type;
        MetricIndexInternal = element.MetricIndexInternal;

        switch( Type )
        {
            case EQUATION_ELEM_IMM_UINT64:
                ImmediateUInt64 = element.ImmediateUInt64;
                break;

            case EQUATION_ELEM_IMM_FLOAT:
                ImmediateFloat = element.ImmediateFloat;
                break;

            case EQUATION_ELEM_OPERATION:
                Operation = element.Operation;
                break;

            case EQUATION_ELEM_RD_BITFIELD:
            case EQUATION_ELEM_RD_UINT8:
            case EQUATION_ELEM_RD_UINT16:
            case EQUATION_ELEM_RD_UINT32:
            case EQUATION_ELEM_RD_UINT64:
            case EQUATION_ELEM_RD_FLOAT:
            case EQUATION_ELEM_RD_40BIT_CNTR:
                ReadParams = element.ReadParams;
                break;

            case EQUATION_ELEM_MASK:
            case EQUATION_ELEM_GLOBAL_SYMBOL:
            case EQUATION_ELEM_LOCAL_COUNTER_SYMBOL:
            case EQUATION_ELEM_OTHER_SET_COUNTER_SYMBOL:
            case EQUATION_ELEM_LOCAL_METRIC_SYMBOL:
            case EQUATION_ELEM_OTHER_SET_METRIC_SYMBOL:
            case EQUATION_ELEM_INFORMATION_SYMBOL:
            case EQUATION_ELEM_PREV_METRIC_SYMBOL:
                // Handled in CopyMembers or MoveMembers.
            case EQUATION_ELEM_SELF_COUNTER_VALUE:
            case EQUATION_ELEM_STD_NORM_GPU_DURATION:
            case EQUATION_ELEM_STD_NORM_EU_AGGR_DURATION:
                // A type describes the equation element, no additional data to copy.
                break;

            default:
                MD_LOG( LOG_ERROR, "Unrecognized element type: %u", Type );
                break;
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquationElementInternal
    //
    // Method:
    //     CopyMembers
    //
    // Description:
    //     Copies dynamically allocated members.
    //
    //////////////////////////////////////////////////////////////////////////////
    void CEquationElementInternal::CopyMembers( const CEquationElementInternal& element )
    {
        SymbolName = GetCopiedCString( element.SymbolName, IU_ADAPTER_ID_UNKNOWN );

        if( element.Type == EQUATION_ELEM_MASK )
        {
            if( const uint32_t byteArraySize = element.Mask.Size;
                byteArraySize != 0 )
            {
                if( element.Mask.Data == nullptr )
                {
                    Mask.Size = 0;
                    Mask.Data = nullptr;
                    MD_LOG( LOG_WARNING, "Cannot copy null element's mask" );
                }
                else
                {
                    Mask.Size = byteArraySize;
                    Mask.Data = new( std::nothrow ) uint8_t[byteArraySize]();

                    if( Mask.Data == nullptr )
                    {
                        MD_LOG( LOG_WARNING, "Cannot allocate memory for element's mask" );
                    }
                    else
                    {
                        iu_memcpy_s( Mask.Data, byteArraySize, element.Mask.Data, byteArraySize );
                    }
                }
            }
            else
            {
                MD_LOG( LOG_DEBUG, "Element's mask has size 0" );
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquationElementInternal
    //
    // Method:
    //     MoveMembers
    //
    // Description:
    //     Moves dynamically allocated members.
    //
    //////////////////////////////////////////////////////////////////////////////
    void CEquationElementInternal::MoveMembers( CEquationElementInternal& element )
    {
        SymbolName         = element.SymbolName; // Move ownership to the new object.
        element.SymbolName = nullptr;            // Delete ownership from the old object.

        if( element.Type == EQUATION_ELEM_MASK )
        {
            if( const uint32_t byteArraySize = element.Mask.Size;
                byteArraySize != 0 )
            {
                if( element.Mask.Data == nullptr )
                {
                    Mask.Size = 0;
                    Mask.Data = nullptr;
                    MD_LOG( LOG_WARNING, "Cannot move null element's mask" );
                }
                else
                {
                    // Move ownership to the new object.
                    Mask.Size = byteArraySize;
                    Mask.Data = element.Mask.Data;

                    // Delete ownership from the old object.
                    element.Mask.Size = 0;
                    element.Mask.Data = nullptr;
                }
            }
            else
            {
                MD_LOG( LOG_DEBUG, "Element's mask has size 0" );
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquation
    //
    // Method:
    //     CEquation constructor
    //
    // Description:
    //     Constructor.
    //
    // Input:
    //     CMetricsDevice& device - parent metric device
    //
    //////////////////////////////////////////////////////////////////////////////
    CEquation::CEquation( CMetricsDevice& device )
        : m_elementsVector()
        , m_equationString( nullptr )
        , m_device( device )
    {
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquation
    //
    // Method:
    //     CEquation copy constructor
    //
    // Description:
    //     Copy constructor.
    //
    //////////////////////////////////////////////////////////////////////////////
    CEquation::CEquation( const CEquation& other )
        : m_elementsVector( other.m_elementsVector )
        , m_equationString( GetCopiedCString( other.m_equationString, other.m_device.GetAdapter().GetAdapterId() ) )
        , m_device( other.m_device )
    {
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquation
    //
    // Method:
    //     CEquation destructor
    //
    // Description:
    //     Deallocates memory.
    //
    //////////////////////////////////////////////////////////////////////////////
    CEquation::~CEquation()
    {
        MD_SAFE_DELETE_ARRAY( m_equationString );
        ClearVector( m_elementsVector );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquation
    //
    // Method:
    //     GetEquationElementsCount
    //
    // Description:
    //     Returns equation elements count.
    //
    // Output:
    //     uint32_t  - equation elements count
    //
    //////////////////////////////////////////////////////////////////////////////
    uint32_t CEquation::GetEquationElementsCount()
    {
        return static_cast<uint32_t>( m_elementsVector.size() );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquation
    //
    // Method:
    //     GetEquationElement
    //
    // Description:
    //     Returns the equation element with given index. Null if doesn't exist.
    //
    // Input:
    //     uint32_t     index      - equation element index
    //
    // Output:
    //     TEquationElement_1_0*   - pointer to the chosen element
    //
    //////////////////////////////////////////////////////////////////////////////
    TEquationElement_1_0* CEquation::GetEquationElement( uint32_t index )
    {
        return ( index < m_elementsVector.size() )
            ? static_cast<TEquationElement_1_0*>( &m_elementsVector[index] )
            : nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquation
    //
    // Method:
    //     SolveBooleanEquation
    //
    // Description:
    //     Used only for availability equations.
    //
    // Output:
    //     bool    -   result of the solved boolean equation
    //
    //////////////////////////////////////////////////////////////////////////////
    bool CEquation::SolveBooleanEquation( void )
    {
        const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

        std::list<uint64_t> equationStack  = {};
        uint64_t            qwordValue     = 0ULL;
        uint32_t            algorithmCheck = 0;
        const uint32_t      elementsCount  = static_cast<uint32_t>( m_elementsVector.size() );

        for( uint32_t i = 0; i < elementsCount; ++i )
        {
            const auto& element = m_elementsVector[i];
            switch( element.Type )
            {
                case EQUATION_ELEM_IMM_UINT64:
                    equationStack.push_back( element.ImmediateUInt64 );
                    algorithmCheck++;
                    break;

                case EQUATION_ELEM_LOCAL_COUNTER_SYMBOL:
                {
                    // Push 0 to stack for unavailable unpacked mask symbol.
                    const bool isUnpackedMaskSymbol = ( element.SymbolName != nullptr ) &&
                        ( ( strstr( element.SymbolName, "GtSlice" ) != nullptr ) ||
                            ( strstr( element.SymbolName, "GtXeCore" ) != nullptr ) ||
                            ( strstr( element.SymbolName, "GtL3Bank" ) != nullptr ) ||
                            ( strstr( element.SymbolName, "GtL3Node" ) != nullptr ) ||
                            ( strstr( element.SymbolName, "GtSqidi" ) != nullptr ) ||
                            ( strstr( element.SymbolName, "GtCopyEngine" ) != nullptr ) ) &&
                        ( strstr( element.SymbolName, "Mask" ) == nullptr );

                    if( isUnpackedMaskSymbol )
                    {
                        qwordValue = 0;
                        equationStack.push_back( qwordValue );
                        algorithmCheck++;
                    }
                    else
                    {
                        MD_LOG_A( adapterId, LOG_DEBUG, "Not allowed equation element type in availability equation: %u", element.Type );
                        MD_ASSERT_A( adapterId, false );
                        ClearList( equationStack );
                        return false;
                    }
                    break;
                }

                case EQUATION_ELEM_GLOBAL_SYMBOL:
                {
                    if( const auto pValue = m_device.GetGlobalSymbolValueByName( element.SymbolName );
                        pValue )
                    {
                        switch( pValue->ValueType )
                        {
                            case VALUE_TYPE_UINT64:
                                qwordValue = static_cast<uint64_t>( pValue->ValueUInt64 );
                                break;

                            case VALUE_TYPE_UINT32:
                                qwordValue = static_cast<uint64_t>( pValue->ValueUInt32 );
                                break;

                            case VALUE_TYPE_BOOL:
                                qwordValue = static_cast<uint64_t>( pValue->ValueBool );
                                break;

                            case VALUE_TYPE_BYTEARRAY:
                                // TODO: should be improved (the array can be bigger than 64bits)
                                if( pValue->ValueByteArray->Size == sizeof( uint64_t ) )
                                {
                                    qwordValue = *reinterpret_cast<uint64_t*>( pValue->ValueByteArray->Data );
                                    break;
                                }
                                [[fallthrough]];

                            default:
                                MD_ASSERT_A( adapterId, false );
                                qwordValue = 0ULL;
                                break;
                        }
                    }
                    else
                    {
                        MD_ASSERT_A( adapterId, false );
                        qwordValue = 0ULL;
                    }

                    equationStack.push_back( qwordValue );
                    algorithmCheck++;
                    break;
                }

                case EQUATION_ELEM_OPERATION:
                {
                    if( equationStack.size() < 2 )
                    {
                        MD_LOG_A( adapterId, LOG_DEBUG, "Not enough elements in equationStack, size is less than 2." );
                        return false;
                    }
                    // Pop two values from stack
                    const uint64_t valueLast = equationStack.back();
                    equationStack.pop_back();
                    algorithmCheck--;
                    const uint64_t valuePrev = equationStack.back();
                    equationStack.pop_back();
                    algorithmCheck--;

                    switch( element.Operation )
                    {
                        case EQUATION_OPER_AND:
                            qwordValue = valuePrev & valueLast;
                            break;

                        case EQUATION_OPER_OR:
                            qwordValue = valuePrev | valueLast;
                            break;

                        case EQUATION_OPER_XOR:
                            qwordValue = valuePrev ^ valueLast;
                            break;

                        case EQUATION_OPER_XNOR:
                            qwordValue = ~( valuePrev ^ valueLast );
                            break;

                        case EQUATION_OPER_EQUALS:
                            qwordValue = valuePrev == valueLast;
                            break;

                        case EQUATION_OPER_AND_L:
                            qwordValue = valuePrev && valueLast;
                            break;

                        case EQUATION_OPER_RSHIFT:
                            qwordValue = valuePrev >> valueLast;
                            break;

                        case EQUATION_OPER_LSHIFT:
                            qwordValue = valuePrev << valueLast;
                            break;

                        case EQUATION_OPER_UADD:
                            qwordValue = valuePrev + valueLast;
                            break;

                        case EQUATION_OPER_USUB:
                            qwordValue = valuePrev - valueLast;
                            break;

                        case EQUATION_OPER_UDIV:
                            qwordValue = ( valueLast != 0 ) ? valuePrev / valueLast : 0;
                            break;

                        case EQUATION_OPER_UMUL:
                            qwordValue = valuePrev * valueLast;
                            break;

                        case EQUATION_OPER_UGT:
                            qwordValue = valuePrev > valueLast;
                            break;

                        case EQUATION_OPER_ULT:
                            qwordValue = valuePrev < valueLast;
                            break;

                        case EQUATION_OPER_UGTE:
                            qwordValue = valuePrev >= valueLast;
                            break;

                        case EQUATION_OPER_ULTE:
                            qwordValue = valuePrev <= valueLast;
                            break;

                        default:
                            MD_LOG_A( adapterId, LOG_DEBUG, "Not allowed equation element operation in availability equation: %u", element.Operation );
                            MD_ASSERT_A( adapterId, false );
                            ClearList( equationStack );
                            return false;
                    }
                    equationStack.push_back( qwordValue );
                    algorithmCheck++;
                    break;
                }

                default:
                    MD_LOG_A( adapterId, LOG_DEBUG, "Not allowed equation element type in availability equation: %u", element.Type );
                    MD_ASSERT_A( adapterId, false );
                    ClearList( equationStack );
                    return false;
            }
        }
        if( elementsCount > 0 )
        {
            // here should be only 1 element on the list - the result (if the equation is fine)
            MD_ASSERT_A( adapterId, algorithmCheck == 1 );
            qwordValue = ( equationStack.size() > 0 ) ? equationStack.back() : 0LL;
        }
        else
        {
            qwordValue = 0LL;
        }
        ClearList( equationStack );

        return ( qwordValue != 0LL );
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquation
    //
    // Method:
    //     ParseEquationString
    //
    // Description:
    //     Parses the equation string.
    //
    // Input:
    //     const char * equationString - equation string to parse
    //
    // Output:
    //     bool                        - result of the operation
    //
    //////////////////////////////////////////////////////////////////////////////
    bool CEquation::ParseEquationString( const char* equationString )
    {
        if( equationString == nullptr || ( strcmp( equationString, "" ) == 0 ) )
        {
            m_equationString = nullptr;
            return false;
        }

        const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

        char* tokenNext = nullptr;

        char* string = GetCopiedCString( equationString, adapterId );
        MD_CHECK_PTR_RET_A( adapterId, string, false );

        char* token = iu_strtok_s( string, " ", &tokenNext );
        while( token != nullptr )
        {
            if( !ParseEquationElement( token ) )
            {
                MD_SAFE_DELETE_ARRAY( string );
                return false;
            }
            token = iu_strtok_s( nullptr, " ", &tokenNext );
        }

        m_equationString = GetCopiedCString( equationString, adapterId );
        MD_SAFE_DELETE_ARRAY( string );
        return true;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquation
    //
    // Method:
    //     ParseEquationElement
    //
    // Description:
    //     Parses and adds the element to the equation list.
    //
    // Input:
    //     const char* equationString - equation element string to parse;
    //                                  expected to be nullptr-terminated
    //
    // Output:
    //     bool                       - result of the operation
    //
    //////////////////////////////////////////////////////////////////////////////
    bool CEquation::ParseEquationElement( const char* equationString )
    {
        const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

        CEquationElementInternal element = {};

        if( strcmp( equationString, "EuAggrDurationSlice" ) == 0 )
        {
            switch( m_device.GetPlatformIndex() )
            {
                case GENERATION_MTL:
                case GENERATION_ARL:
                case GENERATION_ACM:
                case GENERATION_PVC:
                case GENERATION_BMG:
                case GENERATION_LNL:
                case GENERATION_PTL:
                    return ParseEquationString( "$Self $GpuSliceClocksCount $VectorEngineTotalCount UMUL FDIV 100 FMUL" );

                default:
                    return ParseEquationString( "$Self $GpuSliceClocksCount $EuCoresTotalCount UMUL FDIV 100 FMUL" );
            }
        }
        else if( strcmp( equationString, "EuAggrDuration" ) == 0 )
        {
            element.Type = EQUATION_ELEM_STD_NORM_EU_AGGR_DURATION;
        }
        else if( strcmp( equationString, "GpuDurationSlice" ) == 0 )
        {
            return ParseEquationString( "$Self $GpuSliceClocksCount FDIV 100 FMUL" );
        }
        else if( strcmp( equationString, "GpuDuration" ) == 0 )
        {
            element.Type = EQUATION_ELEM_STD_NORM_GPU_DURATION;
        }
        else if( strcmp( equationString, "UADD" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_UADD;
        }
        else if( strcmp( equationString, "USUB" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_USUB;
        }
        else if( strcmp( equationString, "UMUL" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_UMUL;
        }
        else if( strcmp( equationString, "UDIV" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_UDIV;
        }
        else if( strcmp( equationString, "AND" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_AND;
        }
        else if( strcmp( equationString, "OR" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_OR;
        }
        else if( strcmp( equationString, "XNOR" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_XNOR;
        }
        else if( strcmp( equationString, "XOR" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_XOR;
        }
        else if( strcmp( equationString, "==" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_EQUALS;
        }
        else if( strcmp( equationString, "&&" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_AND_L;
        }
        else if( strcmp( equationString, "<<" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_LSHIFT;
        }
        else if( strcmp( equationString, ">>" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_RSHIFT;
        }
        else if( strcmp( equationString, "FADD" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_FADD;
        }
        else if( strcmp( equationString, "FSUB" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_FSUB;
        }
        else if( strcmp( equationString, "FMUL" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_FMUL;
        }
        else if( strcmp( equationString, "FDIV" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_FDIV;
        }
        else if( strcmp( equationString, "UGTE" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_UGTE;
        }
        else if( strcmp( equationString, "ULTE" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_ULTE;
        }
        else if( strcmp( equationString, "UGT" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_UGT;
        }
        else if( strcmp( equationString, "ULT" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_ULT;
        }
        else if( strcmp( equationString, "FGTE" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_FGTE;
        }
        else if( strcmp( equationString, "FLTE" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_FLTE;
        }
        else if( strcmp( equationString, "FGT" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_FGT;
        }
        else if( strcmp( equationString, "FLT" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_FLT;
        }
        else if( strcmp( equationString, "UMIN" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_UMIN;
        }
        else if( strcmp( equationString, "UMAX" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_UMAX;
        }
        else if( strcmp( equationString, "FMIN" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_FMIN;
        }
        else if( strcmp( equationString, "FMAX" ) == 0 )
        {
            element.Type      = EQUATION_ELEM_OPERATION;
            element.Operation = EQUATION_OPER_FMAX;
        }
        else if( strncmp( equationString, "dw@", sizeof( "dw@" ) - 1 ) == 0 )
        {
            element.Type                  = EQUATION_ELEM_RD_UINT32;
            element.ReadParams.ByteOffset = strtoul( &equationString[3], nullptr, 0 );
        }
        else if( strncmp( equationString, "fl@", sizeof( "fl@" ) - 1 ) == 0 )
        {
            element.Type = EQUATION_ELEM_RD_FLOAT;
        }
        else if( strncmp( equationString, "qw@", sizeof( "qw@" ) - 1 ) == 0 )
        {
            element.Type                  = EQUATION_ELEM_RD_UINT64;
            element.ReadParams.ByteOffset = strtoul( &equationString[3], nullptr, 0 );
        }
        else if( strncmp( equationString, "rd8@", sizeof( "rd8@" ) - 1 ) == 0 )
        {
            element.Type                  = EQUATION_ELEM_RD_UINT8;
            element.ReadParams.ByteOffset = strtoul( &equationString[4], nullptr, 0 );
        }
        else if( strncmp( equationString, "rd16@", sizeof( "rd16@" ) - 1 ) == 0 )
        {
            element.Type                  = EQUATION_ELEM_RD_UINT16;
            element.ReadParams.ByteOffset = strtoul( &equationString[5], nullptr, 0 );
        }
        else if( strncmp( equationString, "rd40@", sizeof( "rd40@" ) - 1 ) == 0 )
        {
            char* pEnd                    = (char*) &equationString[5];
            element.Type                  = EQUATION_ELEM_RD_40BIT_CNTR;
            element.ReadParams.ByteOffset = strtoul( pEnd, &pEnd, 0 );
            if( pEnd == nullptr )
            {
                return false;
            }
            element.ReadParams.ByteOffsetExt = strtoul( ++pEnd, &pEnd, 0 );
        }
        else if( strncmp( equationString, "bm@", sizeof( "bm@" ) - 1 ) == 0 )
        {
            char* pEnd                    = (char*) &equationString[3];
            element.Type                  = EQUATION_ELEM_RD_BITFIELD;
            element.ReadParams.ByteOffset = strtoul( pEnd, &pEnd, 0 );
            if( pEnd == nullptr )
            {
                return false;
            }
            element.ReadParams.BitOffset = strtoul( ++pEnd, &pEnd, 0 );
            if( pEnd == nullptr )
            {
                return false;
            }
            element.ReadParams.BitsCount = strtoul( ++pEnd, &pEnd, 10 );
        }
        else if( strcmp( equationString, "$Self" ) == 0 )
        {
            element.Type = EQUATION_ELEM_SELF_COUNTER_VALUE;
        }
        else if( strncmp( equationString, "$$", sizeof( "$$" ) - 1 ) == 0 )
        {
            MD_SAFE_DELETE_ARRAY( element.SymbolName );
            element.SymbolName = GetCopiedCString( &equationString[2], adapterId );
            element.Type       = EQUATION_ELEM_LOCAL_METRIC_SYMBOL;
        }
        else if( strncmp( equationString, "prev$$", sizeof( "prev$$" ) - 1 ) == 0 )
        {
            MD_SAFE_DELETE_ARRAY( element.SymbolName );
            element.SymbolName = GetCopiedCString( &equationString[6], adapterId );
            element.Type       = EQUATION_ELEM_PREV_METRIC_SYMBOL;
        }
        else if( ( equationString[0] == '$' ) && ( equationString[1] != 0 ) )
        {
            std::string symbolName = &equationString[1];

            if( IsLegacyMaskGlobalSymbol( symbolName.c_str() ) )
            {
                // Legacy mask global symbol needs to be prefixed with "Gt"
                symbolName.insert( 0, "Gt" );
            }

            auto value = m_device.GetGlobalSymbolValueByName( symbolName.c_str() );

            MD_SAFE_DELETE_ARRAY( element.SymbolName );
            element.SymbolName = GetCopiedCString( symbolName.c_str(), adapterId );

            element.Type = ( value == nullptr )
                ? EQUATION_ELEM_LOCAL_COUNTER_SYMBOL // Finish element as local counter symbol
                : EQUATION_ELEM_GLOBAL_SYMBOL;       // Finish element as global symbol
        }
        else if( strncmp( equationString, "i$", sizeof( "i$" ) - 1 ) == 0 )
        {
            MD_SAFE_DELETE_ARRAY( element.SymbolName );
            element.SymbolName = GetCopiedCString( &equationString[2], adapterId );
            element.Type       = EQUATION_ELEM_INFORMATION_SYMBOL;
        }
        else if( strchr( equationString, '.' ) != nullptr ) // assume float number
        {
            element.Type           = EQUATION_ELEM_IMM_FLOAT;
            element.ImmediateFloat = static_cast<float>( atof( equationString ) );
        }
        else if( strncmp( equationString, "0x", sizeof( "0x" ) - 1 ) == 0 ) // assume hex integer 64
        {
            element.Type            = EQUATION_ELEM_IMM_UINT64;
            element.ImmediateUInt64 = strtoull( equationString, nullptr, 0 );
        }
        else if( equationString[0] >= '0' && equationString[0] <= '9' ) // assume decimal integer 64
        {
            element.Type            = EQUATION_ELEM_IMM_UINT64;
            element.ImmediateUInt64 = strtoull( equationString, nullptr, 10 );
        }
        else if( strncmp( equationString, "mask$", sizeof( "mask$" ) - 1 ) == 0 ) // assume hex integer 64
        {
            element.Type = EQUATION_ELEM_MASK;
            element.Mask = GetByteArrayFromCStringMask( equationString + sizeof( "mask$" ) - 1, adapterId );
            MD_CHECK_PTR_RET_A( adapterId, element.Mask.Data, false )
        }
        else
        {
            MD_LOG_A( adapterId, LOG_ERROR, "Unknown equation element: %s", equationString );
            return false;
        }

        m_elementsVector.push_back( std::move( element ) );

        return true;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquation
    //
    // Method:
    //     WriteCEquationToBuffer
    //
    // Description:
    //     Writes equation string to buffer.
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
    TCompletionCode CEquation::WriteCEquationToBuffer( uint8_t* buffer, uint32_t& bufferSize, uint32_t& bufferOffset )
    {
        const uint32_t adapterId = m_device.GetAdapter().GetAdapterId();

        TCompletionCode result = WriteCStringToBuffer( m_equationString, buffer, bufferSize, bufferOffset, adapterId );
        MD_CHECK_CC_RET_A( adapterId, result );

        return CC_OK;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CEquation
    //
    // Method:
    //     IsLegacyMaskGlobalSymbol
    //
    // Description:
    //     Checks if given symbol name is a legacy mask global symbol
    //     (not prefixed with "Gt" and suffixed with "Mask").
    //
    // Input:
    //     const char* name            - symbol name
    //
    // Output:
    //     bool                        - true if name is a legacy mask global symbol
    //
    //////////////////////////////////////////////////////////////////////////////
    bool CEquation::IsLegacyMaskGlobalSymbol( const char* name )
    {
        const char* suffix = "Mask";
        const char* prefix = "Gt";

        size_t nameLength   = strlen( name );
        size_t suffixLength = strlen( suffix );

        if( nameLength >= suffixLength && strcmp( name + nameLength - suffixLength, suffix ) == 0 )
        {
            return strncmp( name, prefix, 2 ) != 0;
        }

        return false;
    }
} // namespace MetricsDiscoveryInternal
