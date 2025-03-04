/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//     File Name:  md_register_set.h

//     Abstract:   C++ Metrics Discovery internal registry set header

#pragma once

#include "md_types.h"

#include <cstdio>
#include <vector>
#include <list>

using namespace MetricsDiscovery;

namespace MetricsDiscoveryInternal
{
    ///////////////////////////////////////////////////////////////////////////////
    // Forward declarations:                                                     //
    ///////////////////////////////////////////////////////////////////////////////
    class CMetricsDevice;
    class CEquation;
    class CMetricsDevice;

    //////////////////////////////////////////////////////////////////////////////
    //
    // Class:
    //     CRegisterSet
    //
    // Description:
    //     Stores configuration registers along with an availability equation and other
    //     information.
    //
    //////////////////////////////////////////////////////////////////////////////
    class CRegisterSet
    {
    public:
        // Constructor & Destructor:
        CRegisterSet( CMetricsDevice& device, uint32_t configId, uint32_t configPriority, TConfigType configType );
        virtual ~CRegisterSet();

        CRegisterSet( const CRegisterSet& )            = delete; // Delete copy-constructor
        CRegisterSet& operator=( const CRegisterSet& ) = delete; // Delete assignment operator

        // Non-API:
        TRegisterSetParams* GetParams();
        TCompletionCode     SetAvailabilityEquation( const char* equationString );
        TRegister*          AddConfigRegister( uint32_t offset, uint32_t value, TRegisterType type );
        bool                IsAvailable();
        TCompletionCode     RegsToVector( std::vector<TRegister*>& regVector );

        TCompletionCode WriteCRegisterSetToBuffer( uint8_t* buffer, uint32_t& bufferSize, uint32_t& bufferOffset );

    private:
        // Variables:
        std::list<TRegister> m_regList;
        TRegisterSetParams   m_params;
        CEquation*           m_availabilityEquation;
        CMetricsDevice&      m_device;
        bool                 m_isAvailable;
    };
} // namespace MetricsDiscoveryInternal
