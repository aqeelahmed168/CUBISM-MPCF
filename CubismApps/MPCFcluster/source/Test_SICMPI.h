/*
 *  Test_SICMPI.h
 *  MPCFcluster
 *
 *  Created by Babak Hejazialhosseini on 5/30/12.
 *  Copyright 2012 ETH Zurich. All rights reserved.
 *
 */
#pragma once

#include <limits>

#include "Test_SteadyStateMPI.h"
#include <Test_SIC.h>

class Test_SICMPI: public Test_SIC
{
	Test_SteadyStateMPI * t_ssmpi;
    
protected:
	int XPESIZE, YPESIZE, ZPESIZE;
    
	G * grid;
	FlowStep_LSRK3MPI<G> * stepper;
    
public:
	bool isroot;
    
	Test_SICMPI(const bool isroot, const int argc, const char ** argv):
    Test_SIC(argc, argv), isroot(isroot)
	{
		t_ssmpi = new Test_SteadyStateMPI(isroot, argc, argv);
	}
    
	void setup()
	{
		if (isroot && VERBOSITY)
		{
			printf("////////////////////////////////////////////////////////////\n");
			printf("///////////               TEST SIC MPI           ///////////\n");
			printf("////////////////////////////////////////////////////////////\n");
		}
        
		_setup_constants();
		t_ssmpi->setup_mpi_constants(XPESIZE, YPESIZE, ZPESIZE);
        
		if (!isroot)
			VERBOSITY = 0;
        
		grid = new G(XPESIZE, YPESIZE, ZPESIZE, BPDX, BPDY, BPDZ);
        
		assert(grid != NULL);
        
		stepper = new FlowStep_LSRK3MPI<G>(*grid, CFL, Simulation_Environment::GAMMA1, Simulation_Environment::GAMMA2, parser, VERBOSITY, &profiler,  Simulation_Environment::PC1, Simulation_Environment::PC2);
        
		if(bRESTART)
		{
			t_ssmpi->restart(*grid);
			t = t_ssmpi->get_time();
			step_id = t_ssmpi->get_stepid();
		}
		else
			_ic(*grid);
	}	
    
	void run()
	{
		if (isroot) printf("HELLO RUN\n");
		bool bLoop = (NSTEPS>0) ? (step_id<NSTEPS) : (fabs(t-TEND) > std::numeric_limits<Real>::epsilon()*1e1);
        
		while(bLoop)
		{
			if (isroot) printf("Step id %d,Time %f\n", step_id, t);
            
			if (step_id%DUMPPERIOD==0)
			{
				std::stringstream streamer;
				streamer<<"data-"<<step_id;;
				t_ssmpi->dump(*grid, step_id, streamer.str());
				t_ssmpi->vp(*grid, step_id, bVP);
			}
            
			if (step_id%SAVEPERIOD==0)
				t_ssmpi->save(*grid, step_id, t);
            
			const Real dt = (*stepper)(TEND-t);
            
			if(step_id%10 == 0 && isroot && step_id > 0)
				profiler.printSummary();
            
			_dumpStatistics(*grid, step_id, t, dt);
            
			t+=dt;
			step_id++;
            
			bLoop = (NSTEPS>0) ? (step_id<NSTEPS) : (fabs(t-TEND) > std::numeric_limits<Real>::epsilon()*1e1);
            
            if (dt==0)
                break;
		}
        
		std::stringstream streamer;
		streamer<<"data-"<<step_id;;
		t_ssmpi->dump(*grid, step_id, streamer.str());
        
		if (isroot) printf("Finishing RUN\n");
		MPI_Finalize();
		exit(0);
	}
};
