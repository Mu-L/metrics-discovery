/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//     File Name:  md_per_platform_preamble.h

//     Abstract:   C++ Metrics Discovery internal header

#pragma once

#if defined( __linux__ )

    #define MD_INCLUDE_Common_METRICS 1

// NOTE:
//    MD_INCLUDE_{platform}_METRICS preprocessor macros may be added to compilation parameters
//    to control the scope of metrics.
//    If none of the macros is defined all platforms with source support will be included
//    By defining any of the macros one will automatically exclude all other platforms

// clang-format off
    #if !( MD_INCLUDE_TGL_GT1_METRICS          || \
           MD_INCLUDE_TGL_GT2_METRICS          || \
           MD_INCLUDE_DG1_METRICS              || \
           MD_INCLUDE_RKL_METRICS              || \
           MD_INCLUDE_ADLP_METRICS             || \
           MD_INCLUDE_ADLS_METRICS             || \
           MD_INCLUDE_ADLN_METRICS             || \
           MD_INCLUDE_ACM_GT1_METRICS          || \
           MD_INCLUDE_ACM_GT2_METRICS          || \
           MD_INCLUDE_ACM_GT3_METRICS          || \
           MD_INCLUDE_PVC_GT1_METRICS          || \
           MD_INCLUDE_PVC_GT2_METRICS          || \
           MD_INCLUDE_MTL_GT2_METRICS          || \
           MD_INCLUDE_MTL_GT3_METRICS          || \
           MD_INCLUDE_BMG_METRICS              || \
           MD_INCLUDE_LNL_METRICS              || \
           MD_INCLUDE_PTL_METRICS              || \
           MD_INCLUDE_ARL_GT1_METRICS          || \
           MD_INCLUDE_ARL_GT2_METRICS )
    // clang-format on

        #define MD_INCLUDE_TGL_GT1_METRICS 1
        #define MD_INCLUDE_TGL_GT2_METRICS 1
        #define MD_INCLUDE_DG1_METRICS     1
        #define MD_INCLUDE_RKL_METRICS     1
        #define MD_INCLUDE_ADLP_METRICS    1
        #define MD_INCLUDE_ADLS_METRICS    1
        #define MD_INCLUDE_ADLN_METRICS    1
        #define MD_INCLUDE_ACM_GT1_METRICS 1
        #define MD_INCLUDE_ACM_GT2_METRICS 1
        #define MD_INCLUDE_ACM_GT3_METRICS 1
        #define MD_INCLUDE_PVC_GT1_METRICS 1
        #define MD_INCLUDE_PVC_GT2_METRICS 1
        #define MD_INCLUDE_MTL_GT2_METRICS 1
        #define MD_INCLUDE_MTL_GT3_METRICS 1
        #define MD_INCLUDE_BMG_METRICS     1
        #define MD_INCLUDE_LNL_METRICS     1
        #define MD_INCLUDE_PTL_METRICS     1
        #define MD_INCLUDE_ARL_GT1_METRICS 1
        #define MD_INCLUDE_ARL_GT2_METRICS 1
    #endif

#endif
