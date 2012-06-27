/*
 *  Tests.h
 *  MPCFnode
 *
 *  Created by Babak Hejazialhosseini  on 6/20/11.
 *  Copyright 2011 ETH Zurich. All rights reserved.
 *
 */
#pragma once

//#include "Test_ShockTube.h"
//typedef BlockLab<FluidBlock, tbb::scalable_allocator> Lab;

#ifdef _USE_CVT_
#include "Test_CVT.h"
typedef BlockLab_CVT<FluidBlock, tbb::scalable_allocator> Lab;

#else

//#include "Test_ShockBubble.h"
//typedef BlockLabBubble<FluidBlock, tbb::scalable_allocator> Lab;

#include "Test_SIC.h"
typedef BlockLabCollapse<FluidBlock, tbb::scalable_allocator> Lab;

#endif


//typedef BlockLabBubbleYZSymmetric<FluidBlock, tbb::scalable_allocator> Lab;

//#include "Test_DoubleMachReflection.h"
//typedef BlockLabDMR<FluidBlock, tbb::scalable_allocator> Lab;

//#include "Test_CVT.h"
//typedef BlockLab_CVT<FluidBlock, tbb::scalable_allocator> Lab;
