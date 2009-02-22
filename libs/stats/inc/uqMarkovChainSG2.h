/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------
 *
 * Copyright (C) 2008 The PECOS Development Team
 *
 * Please see http://pecos.ices.utexas.edu for more information.
 *
 * This file is part of the QUESO Library (Quantification of Uncertainty
 * for Estimation, Simulation and Optimization).
 *
 * QUESO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QUESO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QUESO. If not, see <http://www.gnu.org/licenses/>.
 *
 *--------------------------------------------------------------------------
 *
 * $Id$
 *
 * Brief description of this file: 
 * 
 *--------------------------------------------------------------------------
 *-------------------------------------------------------------------------- */

#ifndef __UQ_MAC_SG2_H__
#define __UQ_MAC_SG2_H__

template <class P_V,class P_M>
double
uqMarkovChainSGClass<P_V,P_M>::targetPdfBarrier(
  const P_V* vecValues,
  const P_V* vecDirection,
        P_V* gradVector,
        P_M* hessianMatrix,
        P_V* hessianEffect) const
{
  double result = 0.;

  bool stayInRoutine = true;
  do {
    const P_V* internalValues    = NULL;
    const P_V* internalDirection = NULL;
          P_V* internalGrad      = NULL;
          P_M* internalHessian   = NULL;
          P_V* internalEffect    = NULL;

    /////////////////////////////////////////////////
    // Broadcast 1 of 3
    /////////////////////////////////////////////////
    // bufferChar[0] = 0 or 1 (vecValues     is NULL or not)
    // bufferChar[1] = 0 or 1 (vecDirection  is NULL or not)
    // bufferChar[2] = 0 or 1 (gradVector    is NULL or not)
    // bufferChar[3] = 0 or 1 (hessianMatrix is NULL or not)
    // bufferChar[4] = 0 or 1 (hessianEffect is NULL or not)
    std::vector<char> bufferChar(5,0);

    if (m_env.subRank() == 0) {
      internalValues    = vecValues;
      internalDirection = vecDirection;
      internalGrad      = gradVector;
      internalHessian   = hessianMatrix;
      internalEffect    = hessianEffect;

      if (internalValues    != NULL) bufferChar[0] = 1;
      if (internalDirection != NULL) bufferChar[1] = 1;
      if (internalGrad      != NULL) bufferChar[2] = 1;
      if (internalHessian   != NULL) bufferChar[3] = 1;
      if (internalEffect    != NULL) bufferChar[4] = 1;
    }

    int count = (int) bufferChar.size();
    int mpiRC = MPI_Bcast ((void *) &bufferChar[0], count, MPI_CHAR, 0, m_env.subComm().Comm());
    UQ_FATAL_TEST_MACRO(mpiRC != MPI_SUCCESS,
                        m_env.rank(),
                        "uqMarkovChainSGClass<P_V,P_M>::targetPdfBarrier()",
                        "failed broadcast 1 of 3");

    if (bufferChar[0] == 1) {
      ///////////////////////////////////////////////
      // Broadcast 2 of 3
      ///////////////////////////////////////////////

      // bufferDouble[0...] = contents for (eventual) vecValues
      std::vector<double> bufferDouble(m_initialPosition.size(),0);

      if (m_env.subRank() == 0) {
        for (unsigned int i = 0; i < internalValues->size(); ++i) {
          bufferDouble[i] = (*internalValues)[i];
        }
      }

      count = (int) bufferDouble.size();
      mpiRC = MPI_Bcast ((void *) &bufferDouble[0], count, MPI_DOUBLE, 0, m_env.subComm().Comm());
      UQ_FATAL_TEST_MACRO(mpiRC != MPI_SUCCESS,
                          m_env.rank(),
                          "uqMarkovChainSGClass<P_V,P_M>::targetPdfBarrier()",
                          "failed broadcast 2 of 3");

      if (m_env.subRank() != 0) {
        P_V tmpVec(m_initialPosition);
        for (unsigned int i = 0; i < internalValues->size(); ++i) {
          tmpVec[i] = bufferDouble[i];
        }
        internalValues = new P_V(tmpVec);
      }

      if (bufferChar[1] == 1) {
        /////////////////////////////////////////////
        // Broadcast 3 of 3
        /////////////////////////////////////////////
        // bufferDouble[0...] = contents for (eventual) vecDirection

        if (m_env.subRank() == 0) {
          for (unsigned int i = 0; i < internalDirection->size(); ++i) {
            bufferDouble[i] = (*internalDirection)[i];
          }
        }

        count = (int) bufferDouble.size();
        mpiRC = MPI_Bcast ((void *) &bufferDouble[0], count, MPI_DOUBLE, 0, m_env.subComm().Comm());
        UQ_FATAL_TEST_MACRO(mpiRC != MPI_SUCCESS,
                            m_env.rank(),
                            "uqMarkovChainSGClass<P_V,P_M>::targetPdfBarrier()",
                            "failed broadcast 3 of 3");

        if (m_env.subRank() != 0) {
          P_V tmpVec(m_initialPosition);
          for (unsigned int i = 0; i < internalDirection->size(); ++i) {
            tmpVec[i] = bufferDouble[i];
          }
          internalDirection = new P_V(tmpVec);
        }
      }

      ///////////////////////////////////////////////
      // All processors now call 'targetPdf()'
      ///////////////////////////////////////////////
      if (m_env.subRank() != 0) {
        if (bufferChar[2] == 1) internalGrad    = new P_V(m_initialPosition);
        if (bufferChar[3] == 1) internalHessian = new P_M(m_initialPosition);
        if (bufferChar[4] == 1) internalEffect  = new P_V(m_initialPosition);
      }
      m_env.subComm().Barrier();

      result = m_targetPdf.minus2LnValue(*internalValues,
                                         internalDirection,
                                         internalGrad,
                                         internalHessian,
                                         internalEffect);
    }

    /////////////////////////////////////////////////
    // Prepare to exit routine or to stay in it
    /////////////////////////////////////////////////
    if (m_env.subRank() == 0) {
      stayInRoutine = false; // Always for processor 0
    }
    else {
      if (internalValues    != NULL) delete internalValues;
      if (internalDirection != NULL) delete vecDirection;
      if (internalGrad      != NULL) delete gradVector;
      if (internalHessian   != NULL) delete hessianMatrix;
      if (internalEffect    != NULL) delete hessianEffect;

      stayInRoutine = (bufferChar[0] == 1);
    }
  } while (stayInRoutine);

  return result;
}

template <class P_V,class P_M>
void
uqMarkovChainSGClass<P_V,P_M>::generateSequence(uqBaseVectorSequenceClass<P_V,P_M>& workingChain)
{
  if (m_env.numProcSubsets() == (unsigned int) m_env.fullComm().NumProc()) {
    UQ_FATAL_TEST_MACRO(m_env.subRank() != 0,
                        m_env.rank(),
                        "uqMarkovChainSGClass<P_V,P_M>::generateSequence()",
                        "there should exist only one processor per processor subset communicator");
    UQ_FATAL_TEST_MACRO(m_initialPosition.numberOfProcessorsRequiredForStorage() != 1,
                        m_env.rank(),
                        "uqMarkovChainSGClass<P_V,P_M>::generateSequence()",
                        "only 1 processor (per processor subset) should be necessary for the storage of a vector");
    proc0GenerateSequence(workingChain);
  }
  else if (m_env.numProcSubsets() < (unsigned int) m_env.fullComm().NumProc()) {
    UQ_FATAL_TEST_MACRO(m_env.fullComm().NumProc()%m_env.numProcSubsets() != 0,
                        m_env.rank(),
                        "uqMarkovChainSGClass<P_V,P_M>::generateSequence()",
                        "total number of processors should be a multiple of the number of processor subsets");
    unsigned int numProcsPerProcSubset = m_env.fullComm().NumProc()/m_env.numProcSubsets();
    UQ_FATAL_TEST_MACRO(m_env.subComm().NumProc() != (int) numProcsPerProcSubset,
                        m_env.rank(),
                        "uqMarkovChainSGClass<P_V,P_M>::generateSequence()",
                        "inconsistent number of processors per processor subset communicator");
    if (m_initialPosition.numberOfProcessorsRequiredForStorage() == 1) {
      double aux = 0.;
      if (m_env.subRank() == 0) proc0GenerateSequence(workingChain);

      // subRank == 0 --> Tell all other processors to exit barrier now that the chain has been fully generated
      // subRank != 0 --> Enter the barrier and wait for processor 0 to decide to call the targetPdf
      aux = targetPdfBarrier(NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
    }
    else if (m_initialPosition.numberOfProcessorsRequiredForStorage() == numProcsPerProcSubset) {
      UQ_FATAL_TEST_MACRO(true,
                          m_env.rank(),
                          "uqMarkovChainSGClass<P_V,P_M>::generateSequence()",
                          "situation not supported yet");
    }
    else {
      UQ_FATAL_TEST_MACRO(true,
                          m_env.rank(),
                          "uqMarkovChainSGClass<P_V,P_M>::generateSequence()",
                          "number of processors required for a vector storage should be equal to the number of processors per processor subset communicator");
    }
  }
  else {
  }

  return;
}
template <class P_V,class P_M>
void
uqMarkovChainSGClass<P_V,P_M>::proc0GenerateSequence(uqBaseVectorSequenceClass<P_V,P_M>& workingChain)
{
  if ((m_env.verbosity() >= 5) && (m_env.rank() == 0)) {
    std::cout << "Entering uqMarkovChainSGClass<P_V,P_M>::proc0GenerateSequence()..."
              << std::endl;
  }

  P_V valuesOf1stPosition(m_initialPosition);
  int iRC = UQ_OK_RC;

  workingChain.setName(m_prefix + "chain");

  if (m_chainType == UQ_MAC_SG_WHITE_NOISE_CHAIN_TYPE) {
    //****************************************************
    // Just generate white noise
    //****************************************************
    generateWhiteNoiseChain(m_chainSize,
                            workingChain);
  }
  else if (m_chainType == UQ_MAC_SG_UNIFORM_CHAIN_TYPE) {
    //****************************************************
    // Just generate uniform    
    //****************************************************
    generateUniformChain(m_chainSize,
                         workingChain);
  }
  else {
    //****************************************************
    // Initialize m_lowerCholProposalCovMatrices[0]
    // Initialize m_proposalCovMatrices[0]
    //****************************************************
#ifdef UQ_USES_TK_CLASS
#else
    iRC = computeInitialCholFactors();
    UQ_FATAL_RC_MACRO(iRC,
                      m_env.rank(),
                      "uqMarkovChainSGClass<P_V,P_M>::proc0GenerateSequence()",
                      "improper computeInitialCholFactors() return");

    if (m_drMaxNumExtraStages > 0) updateTK();
#endif

    //****************************************************
    // Generate chain
    //****************************************************
    generateFullChain(valuesOf1stPosition,
                      workingChain,
                      m_chainSize);
  }

  //****************************************************
  // Open file      
  //****************************************************
  std::ofstream* ofs = NULL;
  if (m_chainOutputFileName == UQ_MAC_SG_FILENAME_FOR_NO_OUTPUT_FILE) {
    if (m_env.rank() == 0) {
      std::cout << "No output file opened for chain " << workingChain.name()
                << std::endl;
    }
  }
  else {
    if (m_env.rank() == 0) {
      std::cout << "Opening output file '"  << m_chainOutputFileName
                << "' for chain " << workingChain.name()
                << std::endl;
    }

    // Open file
#if 0
    // Always write over an eventual pre-existing file
    ofs = new std::ofstream(m_chainOutputFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
#else
    // Always write at the end of an eventual pre-existing file
    ofs = new std::ofstream(m_chainOutputFileName.c_str(), std::ofstream::out | std::ofstream::in | std::ofstream::ate);
    if ((ofs            == NULL ) ||
        (ofs->is_open() == false)) {
      delete ofs;
      ofs = new std::ofstream(m_chainOutputFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
    }
#endif

    UQ_FATAL_TEST_MACRO((ofs && ofs->is_open()) == false,
                        m_env.rank(),
                        "uqMarkovChainSGClass<P_V,P_M>::proc0GenerateSequence()",
                        "failed to open file");
  }
  
  //****************************************************
  // Eventually:
  // --> write chain
  // --> compute statistics on it
  //****************************************************
  if (m_chainWrite && ofs) {
    workingChain.printContents(*ofs);

    // Write likelihoodValues and alphaValues, if they were requested by user
    iRC = writeInfo(workingChain,
                    *ofs);
    UQ_FATAL_RC_MACRO(iRC,
                      m_env.rank(),
                      "uqMarkovChainSGClass<P_V,P_M>::proc0GenerateSequence()",
                      "improper writeInfo() return");
  }

  if (m_chainComputeStats) {
    workingChain.computeStatistics(*m_chainStatisticalOptions,
                                   ofs);
  }

  //****************************************************
  // Eventually:
  // --> generate an unique chain
  // --> write it
  // --> compute statistics on it
  //****************************************************
  if (m_uniqueChainGenerate) {
    // Select only the unique positions
    workingChain.select(m_idsOfUniquePositions);
    //chainVectorPositionIteratorTypedef positionIterator = m_uniqueChain1.begin();
    //std::advance(positionIterator,uniquePos);
    //m_uniqueChain1.erase(positionIterator,m_uniqueChain1.end());
    //UQ_FATAL_TEST_MACRO((uniquePos != m_uniqueChain1.size()),
    //                    m_env.rank(),
    //                    "uqMarkovChainSGClass<P_V,P_M>::proc0GenerateSequence()",
    //                    "uniquePos != m_uniqueChain1.size()");

    // Write unique chain
    workingChain.setName(m_prefix + "uniqueChain");
    if (m_uniqueChainWrite && ofs) {
      workingChain.printContents(*ofs);
    }

    // Compute statistics
    if (m_uniqueChainComputeStats) {
      workingChain.computeStatistics(*m_uniqueChainStatisticalOptions,
                                     ofs);
    }
  }

  //****************************************************
  // Eventually:
  // --> filter the (maybe unique) chain
  // --> write it
  // --> compute statistics on it
  //****************************************************
  if (m_filteredChainGenerate) {
    // Compute filter parameters
    unsigned int filterInitialPos = (unsigned int) (m_filteredChainDiscardedPortion * (double) workingChain.sequenceSize());
    unsigned int filterSpacing    = m_filteredChainLag;
    if (filterSpacing == 0) {
      workingChain.computeFilterParams(*m_filteredChainStatisticalOptions,
                                       ofs,
                                       filterInitialPos,
                                       filterSpacing);
    }

    // Filter positions from the converged portion of the chain
    workingChain.filter(filterInitialPos,
                        filterSpacing);

    // Write filtered chain
    workingChain.setName(m_prefix + "filteredChain");
    if (m_filteredChainWrite && ofs) {
      workingChain.printContents(*ofs);
    }

    // Compute statistics
    if (m_filteredChainComputeStats) {
      workingChain.computeStatistics(*m_filteredChainStatisticalOptions,
                                     ofs);
    }
  }

  //****************************************************
  // Close file      
  //****************************************************
  if (ofs) {
    // Close file
    ofs->close();

    if (m_env.rank() == 0) {
      std::cout << "Closed output file '" << m_chainOutputFileName
                << "' for chain "         << workingChain.name()
                << std::endl;
    }
  }
  if (m_env.rank() == 0) {
    std::cout << std::endl;
  }

  if ((m_env.verbosity() >= 5) && (m_env.rank() == 0)) {
    std::cout << "Leaving uqMarkovChainSGClass<P_V,P_M>::proc0GenerateSequence()"
              << std::endl;
  }

  return;
}

template <class P_V,class P_M>
void
uqMarkovChainSGClass<P_V,P_M>::generateWhiteNoiseChain(
  unsigned int                        chainSize,
  uqBaseVectorSequenceClass<P_V,P_M>& workingChain)
{
  if (m_env.rank() == 0) {
    std::cout << "Starting the generation of white noise chain " << workingChain.name()
              << ", with "                                       << chainSize
              << " positions ..."
              << std::endl;
  }

  int iRC;
  struct timeval timevalTmp;
  double tmpRunTime;

  tmpRunTime = 0.;
  iRC = gettimeofday(&timevalTmp, NULL);
  workingChain.resizeSequence(chainSize); 

  P_V meanVec  (m_vectorSpace.zeroVector());
  P_V stdDevVec(m_vectorSpace.zeroVector());
  meanVec.cwSet(0.);
  stdDevVec.cwSet(1.);
  workingChain.setGaussian(m_env.rng(),meanVec,stdDevVec);

  tmpRunTime += uqMiscGetEllapsedSeconds(&timevalTmp);

  if (m_env.rank() == 0) {
    std::cout << "Finished the generation of white noise chain " << workingChain.name()
              << ", with "                                       << workingChain.sequenceSize()
              << " positions";
  }

  if (m_env.rank() == 0) {
    std::cout << "Chain generation took " << tmpRunTime
              << " seconds"
              << std::endl;
  }

  return;
}

template <class P_V,class P_M>
void
uqMarkovChainSGClass<P_V,P_M>::generateUniformChain(
  unsigned int                        chainSize,
  uqBaseVectorSequenceClass<P_V,P_M>& workingChain)
{
  if (m_env.rank() == 0) {
    std::cout << "Starting the generation of uniform chain " << workingChain.name()
              << ", with "                                   << chainSize
              << " positions ..."
              << std::endl;
  }

  int iRC;
  struct timeval timevalTmp;
  double tmpRunTime;

  tmpRunTime = 0.;
  iRC = gettimeofday(&timevalTmp, NULL);
  workingChain.resizeSequence(chainSize); 

  P_V aVec(m_vectorSpace.zeroVector());
  P_V bVec(m_vectorSpace.zeroVector());
  aVec.cwSet(0.);
  bVec.cwSet(1.);
  workingChain.setUniform(m_env.rng(),aVec,bVec);

  tmpRunTime += uqMiscGetEllapsedSeconds(&timevalTmp);

  if (m_env.rank() == 0) {
    std::cout << "Finished the generation of white noise chain " << workingChain.name()
              << ", with "                                       << workingChain.sequenceSize()
              << " positions";
  }

  if (m_env.rank() == 0) {
    std::cout << "Chain generation took " << tmpRunTime
              << " seconds"
              << std::endl;
  }

  return;
}

template <class P_V,class P_M>
void
uqMarkovChainSGClass<P_V,P_M>::generateFullChain(
  const P_V&                          valuesOf1stPosition,
  uqBaseVectorSequenceClass<P_V,P_M>& workingChain,
  unsigned int                        chainSize)
{
  if (m_env.rank() == 0) {
    std::cout << "Starting the generation of Markov chain " << workingChain.name()
              << ", with "                                  << chainSize
              << " positions..."
              << std::endl;
  }

  int iRC = UQ_OK_RC;
  struct timeval timevalChain;
  struct timeval timevalCandidate;
  struct timeval timevalTargetD;
  struct timeval timevalMhAlpha;
  struct timeval timevalDrAlpha;
  struct timeval timevalDR;
  struct timeval timevalAM;

  double candidateRunTime = 0;
  double targetDRunTime   = 0;
  double mhAlphaRunTime   = 0;
  double drAlphaRunTime   = 0;
  double drRunTime        = 0;
  double amRunTime        = 0;

  iRC = gettimeofday(&timevalChain, NULL);
  bool outOfTargetSupport = !m_targetPdf.domainSet().contains(valuesOf1stPosition);
  UQ_FATAL_TEST_MACRO(outOfTargetSupport,
                      m_env.rank(),
                      "uqMarkovChainSGClass<P_V,P_M>::generateFullChain()",
                      "initial position should not be out of target pdf support");
  if (m_env.rank() == 0) {
    std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
              << ": contents of initial position are:\n";
  }
  std::cout << valuesOf1stPosition;
  if (m_env.rank() == 0) {
    std::cout << std::endl;
  }
  if (m_chainMeasureRunTimes) iRC = gettimeofday(&timevalTargetD, NULL);
  double logTarget = -0.5 * targetPdfBarrier(&valuesOf1stPosition,NULL,NULL,NULL,NULL); // Might demand parallel environment
  if (m_chainMeasureRunTimes) targetDRunTime += uqMiscGetEllapsedSeconds(&timevalTargetD);
  //std::cout << "AQUI 001" << std::endl;
  uqMarkovChainPositionDataClass<P_V> currentPositionData(m_env,
                                                          valuesOf1stPosition,
                                                          outOfTargetSupport,
                                                          logTarget);

  P_V gaussianVector(m_vectorSpace.zeroVector());
  P_V tmpVecValues(m_vectorSpace.zeroVector());
  uqMarkovChainPositionDataClass<P_V> currentCandidateData(m_env);

  //****************************************************
  // Begin chain loop from positionId = 1
  //****************************************************
  workingChain.resizeSequence(chainSize); 
  if (m_uniqueChainGenerate) m_idsOfUniquePositions.resize(chainSize,0); 
  if (m_chainGenerateExtra) {
    m_logTargets.resize (chainSize,0.);
    m_alphaQuotients.resize(chainSize,0.);
  }

  unsigned int uniquePos = 0;
  workingChain.setPositionValues(0,currentPositionData.vecValues());
  if (m_uniqueChainGenerate) m_idsOfUniquePositions[uniquePos++] = 0;
  if (m_chainGenerateExtra) {
    m_logTargets    [0] = currentPositionData.logTarget();
    m_alphaQuotients[0] = 1.;
  }
  //std::cout << "AQUI 002" << std::endl;

  if ((m_env.verbosity() >= 10) && (m_env.rank() == 0)) {
    std::cout << "\n"
              << "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
              << "\n"
              << std::endl;
  }

  for (unsigned int positionId = 1; positionId < workingChain.sequenceSize(); ++positionId) {
    if ((m_env.verbosity() >= 10) && (m_env.rank() == 0)) {
      std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                << ": beginning chain position of id = " << positionId
                << ", m_drMaxNumExtraStages =  "         << m_drMaxNumExtraStages
                << std::endl;
    }
    unsigned int stageId = 0;

#ifdef UQ_USES_TK_CLASS
    m_tk->clearPreComputingPositions();

    if ((m_env.verbosity() >= 5) && (m_env.rank() == 0)) {
      std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                << ": about to set TK pre computing position of local id " << 0
                << ", values = " << currentPositionData.vecValues()
                << std::endl;
    }
    bool validPreComputingPosition = m_tk->setPreComputingPosition(currentPositionData.vecValues(),0);
    if ((m_env.verbosity() >= 5) && (m_env.rank() == 0)) {
      std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                << ": returned from setting TK pre computing position of local id " << 0
                << ", values = " << currentPositionData.vecValues()
                << ", valid = "  << validPreComputingPosition
                << std::endl;
    }
    UQ_FATAL_TEST_MACRO(validPreComputingPosition == false,
                        m_env.rank(),
                        "uqMarkovChainSGClass<P_V,P_M>::generateFullChain()",
                        "initial position should not be an invalid pre computing position");
#endif

    //****************************************************
    // Loop: generate new position
    //****************************************************
    if (m_chainMeasureRunTimes) iRC = gettimeofday(&timevalCandidate, NULL);
#ifdef UQ_USES_TK_CLASS
    m_tk->rv(0).realizer().realization(tmpVecValues);

    if ((m_env.verbosity() >= 5) && (m_env.rank() == 0)) {
      std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                << ": about to set TK pre computing position of local id " << stageId+1
                << ", values = " << tmpVecValues
                << std::endl;
    }
    validPreComputingPosition = m_tk->setPreComputingPosition(tmpVecValues,stageId+1);
    if ((m_env.verbosity() >= 5) && (m_env.rank() == 0)) {
      std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                << ": returned from setting TK pre computing position of local id " << stageId+1
                << ", values = " << tmpVecValues
                << ", valid = "  << validPreComputingPosition
                << std::endl;
    }
#else
    gaussianVector.cwSetGaussian(m_env.rng(),0.,1.);
    tmpVecValues = currentPositionData.vecValues() + *(m_lowerCholProposalCovMatrices[stageId]) * gaussianVector;
#endif
    if (m_chainMeasureRunTimes) candidateRunTime += uqMiscGetEllapsedSeconds(&timevalCandidate);


    outOfTargetSupport = !m_targetPdf.domainSet().contains(tmpVecValues);
    if (outOfTargetSupport) {
      m_numOutOfTargetSupport++;
      logTarget = -INFINITY;
    }
    else {
      if (m_chainMeasureRunTimes) iRC = gettimeofday(&timevalTargetD, NULL);
      logTarget = -0.5 * targetPdfBarrier(&tmpVecValues,NULL,NULL,NULL,NULL); // Might demand parallel environment
      if (m_chainMeasureRunTimes) targetDRunTime += uqMiscGetEllapsedSeconds(&timevalTargetD);
    }
    currentCandidateData.set(tmpVecValues,
                             outOfTargetSupport,
                             logTarget);

    if ((m_env.verbosity() >= 10) && (m_env.rank() == 0)) {
      std::cout << "\n"
                << "\n-----------------------------------------------------------\n"
                << "\n"
                << std::endl;
    }
    bool accept = false;
    double alphaFirstCandidate = 0.;
    if (outOfTargetSupport) {
      if (m_chainGenerateExtra) {
        m_alphaQuotients[positionId] = 0.;
      }
    }
    else {
      if (m_chainMeasureRunTimes) iRC = gettimeofday(&timevalMhAlpha, NULL);
      if (m_chainGenerateExtra) {
        alphaFirstCandidate = this->alpha(currentPositionData,currentCandidateData,0,1,&m_alphaQuotients[positionId]);
      }
      else {
        alphaFirstCandidate = this->alpha(currentPositionData,currentCandidateData,0,1,NULL);
      }
      if (m_chainMeasureRunTimes) mhAlphaRunTime += uqMiscGetEllapsedSeconds(&timevalMhAlpha);
      if ((m_env.verbosity() >= 10) && (m_env.rank() == 0)) {
        std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                  << ": for chain position of id = " << positionId
                  << std::endl;
      }
      accept = acceptAlpha(alphaFirstCandidate);
    }
    if (m_env.verbosity() >= 10) {
      if (m_env.rank() == 0) std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                                       << ": for chain position of id = " << positionId
                                       << ", outOfTargetSupport = "       << outOfTargetSupport
                                       << ", alpha = "                    << alphaFirstCandidate
                                       << ", accept = "                   << accept
                                       << ", currentCandidateData.vecValues() = ";
      std::cout << currentCandidateData.vecValues();
      if (m_env.rank() == 0) std::cout << "\n"
                                       << "\n curLogTarget  = "           << currentPositionData.logTarget()
                                       << "\n"
                                       << "\n canLogTarget  = "           << currentCandidateData.logTarget()
                                       << "\n"
                                       << std::endl;
    }
    if ((m_env.verbosity() >= 10) && (m_env.rank() == 0)) {
      std::cout << "\n"
                << "\n-----------------------------------------------------------\n"
                << "\n"
                << std::endl;
    }

    //****************************************************
    // Loop: delayed rejection
    //****************************************************
    std::vector<uqMarkovChainPositionDataClass<P_V>*> drPositionsData(stageId+2,NULL);
    std::vector<unsigned int> tkStageIds (stageId+2,0);
    if ((accept == false) && (outOfTargetSupport == false) && (m_drMaxNumExtraStages > 0)) {
      if (m_chainMeasureRunTimes) iRC = gettimeofday(&timevalDR, NULL);

      drPositionsData[0] = new uqMarkovChainPositionDataClass<P_V>(currentPositionData );
      drPositionsData[1] = new uqMarkovChainPositionDataClass<P_V>(currentCandidateData);

      tkStageIds[0]  = 0;
      tkStageIds[1]  = 1;

      while ((validPreComputingPosition == true ) && 
             (accept                    == false) &&
             (stageId < m_drMaxNumExtraStages   )) {
        if ((m_env.verbosity() >= 10) && (m_env.rank() == 0)) {
          std::cout << "\n"
                    << "\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-"
                    << "\n"
                    << std::endl;
        }
        stageId++;
        if ((m_env.verbosity() >= 10) && (m_env.rank() == 0)) {
          std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                    << ": for chain position of id = " << positionId
                    << ", beginning stageId = "        << stageId
                    << std::endl;
        }

        if (m_chainMeasureRunTimes) iRC = gettimeofday(&timevalCandidate, NULL);
#ifdef UQ_USES_TK_CLASS
        m_tk->rv(tkStageIds).realizer().realization(tmpVecValues);
        if ((m_env.verbosity() >= 5) && (m_env.rank() == 0)) {
          std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                    << ": about to set TK pre computing position of local id " << stageId+1
                    << ", values = " << tmpVecValues
                    << std::endl;
        }
        validPreComputingPosition = m_tk->setPreComputingPosition(tmpVecValues,stageId+1);
        if ((m_env.verbosity() >= 5) && (m_env.rank() == 0)) {
          std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                    << ": returned from setting TK pre computing position of local id " << stageId+1
                    << ", values = " << tmpVecValues
                    << ", valid = "  << validPreComputingPosition
                    << std::endl;
        }
#else
        gaussianVector.cwSetGaussian(m_env.rng(),0.,1.);
        tmpVecValues = currentPositionData.vecValues() + *(m_lowerCholProposalCovMatrices[stageId]) * gaussianVector;
#endif
        if (m_chainMeasureRunTimes) candidateRunTime += uqMiscGetEllapsedSeconds(&timevalCandidate);

        outOfTargetSupport = !m_targetPdf.domainSet().contains(tmpVecValues);
        if (outOfTargetSupport) {
          logTarget = -INFINITY;
        }
        else {
          if (m_chainMeasureRunTimes) iRC = gettimeofday(&timevalTargetD, NULL);
          logTarget = -0.5 * targetPdfBarrier(&tmpVecValues,NULL,NULL,NULL,NULL); // Might demand parallel environment
          if (m_chainMeasureRunTimes) targetDRunTime += uqMiscGetEllapsedSeconds(&timevalTargetD);
        }
        currentCandidateData.set(tmpVecValues,
                                 outOfTargetSupport,
                                 logTarget);

        drPositionsData.push_back(new uqMarkovChainPositionDataClass<P_V>(currentCandidateData));
        tkStageIds.push_back     (stageId+1);

        double alphaDR = 0.;
        if (outOfTargetSupport == false) {
          if (m_chainMeasureRunTimes) iRC = gettimeofday(&timevalDrAlpha, NULL);
          alphaDR = this->alpha(drPositionsData,tkStageIds);
          if (m_chainMeasureRunTimes) drAlphaRunTime += uqMiscGetEllapsedSeconds(&timevalDrAlpha);
          accept = acceptAlpha(alphaDR);
        }
        if (m_env.verbosity() >= 10) {
          if (m_env.rank() == 0) std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                                           << ": for chain position of id = " << positionId
                                           << " and stageId = "               << stageId
                                           << ", outOfTargetSupport = "       << outOfTargetSupport
                                           << ", alpha = "                    << alphaDR
                                           << ", accept = "                   << accept
                                           << ", currentCandidateData.vecValues() = ";
          std::cout << currentCandidateData.vecValues();
          if (m_env.rank() == 0) std::cout << std::endl;
        }
      } // while

      if (m_chainMeasureRunTimes) drRunTime += uqMiscGetEllapsedSeconds(&timevalDR);
    } // end of 'delayed rejection' logic

    for (unsigned int i = 0; i < drPositionsData.size(); ++i) {
      if (drPositionsData[i]) delete drPositionsData[i];
    }

    //****************************************************
    // Loop: update chain
    //****************************************************
    if (accept) {
      workingChain.setPositionValues(positionId,currentCandidateData.vecValues());
      if (m_uniqueChainGenerate) m_idsOfUniquePositions[uniquePos++] = positionId;
      currentPositionData = currentCandidateData;
    }
    else {
      workingChain.setPositionValues(positionId,currentPositionData.vecValues());
      m_numRejections++;
    }

    if (m_chainGenerateExtra) {
      m_logTargets[positionId] = currentPositionData.logTarget();
    }

#ifdef UQ_MAC_SG_REQUIRES_TARGET_DISTRIBUTION_ONLY
#else
    if (m_likelihoodObjComputesMisfits) {
      if (m_observableSpace.shouldVariancesBeUpdated()) {
        L_V misfitVec    (currentPositionData.misfitVector()       );
        L_V numbersOfObs (m_observableSpace.numbersOfObservations());
        L_V varAccuracies(m_observableSpace.varianceAccuracies()   );
        L_V priorVars    (m_observableSpace.priorVariances()       );
        for (unsigned int i = 0; i < misfitVarianceVector.size(); ++i) {
          double term1 = 0.5*( varAccuracies[i] + numbersOfObs[i]             );
          double term2 =  2./( varAccuracies[i] * priorVars[i] + misfitVec[i] );
          misfitVarianceVector[i] = 1./uqMiscGammar(term1,term2,m_env.rng());
          //if (m_env.rank() == 0) {
          //  std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
          //            << ": for chain position of id = "     << positionId
          //            << ", numbersOfObs = "                 << numbersOfObs
          //            << ", varAccuracies = "                << varAccuracies
          //            << ", priorVars = "                    << priorVars
          //            << ", (*m_misfitChain[positionId]) = " << (*m_misfitChain[positionId])
          //            << ", term1 = "                        << term1
          //            << ", term2 = "                        << term2
          //            << std::endl;
          //}
        }
        //if (m_env.rank() == 0) {
        //  std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
        //            << ": for chain position of id = "        << positionId
        //            << ", misfitVarianceVector changed from " << *(m_misfitVarianceChain[positionId])
        //            << " to "                                 << misfitVarianceVector
        //            << std::endl;
        //}
      }
      if (m_chainGenerateExtra) {
        m_misfitVarianceChain[positionId] = m_observableSpace.newVector(misfitVarianceVector);
      }
    }
#endif

    //****************************************************
    // Loop: adaptive Metropolis (adaptation of covariance matrix)
    //****************************************************
    if ((m_tkUseLocalHessian ==    false) && // IMPORTANT
        (m_amInitialNonAdaptInterval > 0) &&
        (m_amAdaptInterval           > 0)) {
      if (m_chainMeasureRunTimes) iRC = gettimeofday(&timevalAM, NULL);

      // Now might be the moment to adapt
      unsigned int idOfFirstPositionInSubChain = 0;
      uqSequenceOfVectorsClass<P_V,P_M> subChain(m_vectorSpace,0,m_prefix+"subChain");

      // Check if now is indeed the moment to adapt
      if (positionId < m_amInitialNonAdaptInterval) {
        // Do nothing
      }
      else if (positionId == m_amInitialNonAdaptInterval) {
        idOfFirstPositionInSubChain = 0;
        subChain.resizeSequence(m_amInitialNonAdaptInterval+1);
        m_lastMean             = m_vectorSpace.newVector();
        m_lastAdaptedCovMatrix = m_vectorSpace.newMatrix();
      }
      else {
        unsigned int interval = positionId - m_amInitialNonAdaptInterval;
        if ((interval % m_amAdaptInterval) == 0) {
          idOfFirstPositionInSubChain = positionId - m_amAdaptInterval;
          subChain.resizeSequence(m_amAdaptInterval);
        }
      }

      // If now is indeed the moment to adapt, then do it!
      if (subChain.sequenceSize() > 0) {
        P_V transporterVec(m_vectorSpace.zeroVector());
        for (unsigned int i = 0; i < subChain.sequenceSize(); ++i) {
          workingChain.getPositionValues(idOfFirstPositionInSubChain+i,transporterVec);
          subChain.setPositionValues(i,transporterVec);
        }
        updateAdaptedCovMatrix(subChain,
                               idOfFirstPositionInSubChain,
                               m_lastChainSize,
                              *m_lastMean,
                              *m_lastAdaptedCovMatrix);

        bool tmpCholIsPositiveDefinite = false;
        P_M tmpChol(*m_lastAdaptedCovMatrix);
        P_M attemptedMatrix(tmpChol);
        //if (m_env.rank() == 0) {
        //  std::cout << "DRAM"
        //            << ", positionId = "  << positionId
        //            << ": 'am' calling first tmpChol.chol()"
        //            << std::endl;
        //}
        iRC = tmpChol.chol();
        //if (m_env.rank() == 0) {
        //  std::cout << "DRAM"
        //            << ", positionId = "  << positionId
        //            << ": 'am' got first tmpChol.chol() with iRC = " << iRC
        //            << std::endl;
        //}
        if (iRC) {
          UQ_FATAL_TEST_MACRO(iRC != UQ_MATRIX_IS_NOT_POS_DEFINITE_RC,
                              m_env.rank(),
                              "uqMarkovChainSGClass<P_V,P_M>::generateFullChain()",
                              "invalid iRC returned from first chol()");
          // Matrix is not positive definite
          P_M* tmpDiag = m_vectorSpace.newDiagMatrix(m_amEpsilon);
          tmpChol = *m_lastAdaptedCovMatrix + *tmpDiag;
          attemptedMatrix = tmpChol;
          delete tmpDiag;
          //if (m_env.rank() == 0) {
          //  std::cout << "DRAM"
          //            << ", positionId = "  << positionId
          //            << ": 'am' calling second tmpChol.chol()"
          //            << std::endl;
          //}
          iRC = tmpChol.chol();
          //if (m_env.rank() == 0) {
          //  std::cout << "DRAM"
          //            << ", positionId = "  << positionId
          //            << ": 'am' got second tmpChol.chol() with iRC = " << iRC
          //            << std::endl;
          //}
          if (iRC) {
            UQ_FATAL_TEST_MACRO(iRC != UQ_MATRIX_IS_NOT_POS_DEFINITE_RC,
                                m_env.rank(),
                                "uqMarkovChainSGClass<P_V,P_M>::generateFullChain()",
                                "invalid iRC returned from second chol()");
            // Do nothing
          }
          else {
            tmpCholIsPositiveDefinite = true;
          }
        }
        else {
          tmpCholIsPositiveDefinite = true;
        }
        if (tmpCholIsPositiveDefinite) {
#ifdef UQ_USES_TK_CLASS
          uqScaledCovMatrixTKGroupClass<P_V,P_M>* tempTK = dynamic_cast<uqScaledCovMatrixTKGroupClass<P_V,P_M>* >(m_tk);
          tempTK->updateCovMatrix(m_amEta*attemptedMatrix);
#else
          *(m_lowerCholProposalCovMatrices[0]) = tmpChol;
          *(m_lowerCholProposalCovMatrices[0]) *= sqrt(m_amEta);
          m_lowerCholProposalCovMatrices[0]->zeroUpper(false);
#endif

#ifdef UQ_DRAM_MCG_REQUIRES_INVERTED_COV_MATRICES
          UQ_FATAL_RC_MACRO(UQ_INCOMPLETE_IMPLEMENTATION_RC,
                            m_env.rank(),
                            "uqMarkovChainSGClass<P_V,P_M>::generateFullChain()",
                            "need to code the update of m_upperCholProposalPrecMatrices");
#endif

#ifdef UQ_USES_TK_CLASS
#else
          if (m_drMaxNumExtraStages > 0) updateTK();
#endif
        }

        //for (unsigned int i = 0; i < subChain.sequenceSize(); ++i) {
        //  if (subChain[i]) delete subChain[i];
        //}
      }

      if (m_chainMeasureRunTimes) amRunTime += uqMiscGetEllapsedSeconds(&timevalAM);
    } // End of 'adaptive Metropolis' logic

    if ((m_env.verbosity() >= 10) && (m_env.rank() == 0)) {
      std::cout << "In uqMarkovChainSGClass<P_V,P_M>::generateFullChain()"
                << ": finishing chain position of id = " << positionId
                << std::endl;
    }

    if ((m_chainDisplayPeriod                     > 0) && 
        (((positionId+1) % m_chainDisplayPeriod) == 0)) {
      if (m_env.rank() == 0) {
        std::cout << "Finished generating " << positionId+1
                  << " positions"
                  << std::endl;
      }
    }

    if ((m_env.verbosity() >= 10) && (m_env.rank() == 0)) {
      std::cout << "\n"
                << "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                << "\n"
                << std::endl;
    }
  } // end chain loop

  //****************************************************
  // Print basic information about the chain
  //****************************************************
  m_chainRunTime += uqMiscGetEllapsedSeconds(&timevalChain);
  if (m_env.rank() == 0) {
    std::cout << "Finished the generation of Markov chain " << workingChain.name()
              << ", with "                                  << workingChain.sequenceSize()
              << " positions";
    if (m_uniqueChainGenerate) {
      std::cout << " and " << uniquePos
                << " 'unique' positions (i.e., not counting repetitions due to rejections)";
    }
    std::cout << "\nSome information about this chain:"
              << "\n  Chain run time       = " << m_chainRunTime
              << " seconds";
    if (m_chainMeasureRunTimes) {
      std::cout << "\n\n Breaking of the chain run time:\n";
      std::cout << "\n  Candidate run time   = " << candidateRunTime
                << " seconds ("                  << 100.*candidateRunTime/m_chainRunTime
                << "%)";
      std::cout << "\n  Target d. run time   = " << targetDRunTime
                << " seconds ("                  << 100.*targetDRunTime/m_chainRunTime
                << "%)";
      std::cout << "\n  Mh alpha run time    = " << mhAlphaRunTime
                << " seconds ("                  << 100.*mhAlphaRunTime/m_chainRunTime
                << "%)";
      std::cout << "\n  Dr alpha run time    = " << drAlphaRunTime
                << " seconds ("                  << 100.*drAlphaRunTime/m_chainRunTime
                << "%)";
      std::cout << "\n----------------------   --------------";
      double sumRunTime = candidateRunTime + targetDRunTime + mhAlphaRunTime + drAlphaRunTime;
      std::cout << "\n  Sum                  = " << sumRunTime
                << " seconds ("                  << 100.*sumRunTime/m_chainRunTime
                << "%)";
      std::cout << "\n\n Other run times:";
      std::cout << "\n  DR run time          = " << drRunTime
                << " seconds ("                  << 100.*drRunTime/m_chainRunTime
                << "%)";
      std::cout << "\n  AM run time          = " << amRunTime
                << " seconds ("                  << 100.*amRunTime/m_chainRunTime
                << "%)";
    }
    std::cout << "\n  Rejection percentage = "   << 100. * (double) m_numRejections/(double) workingChain.sequenceSize()
              << " %";
    std::cout << "\n  Out of target support percentage = " << 100. * (double) m_numOutOfTargetSupport/(double) workingChain.sequenceSize()
              << " %";
    std::cout << std::endl;
  }

  //****************************************************
  // Release memory before leaving routine
  //****************************************************

  return;
}

template <class P_V,class P_M>
void
uqMarkovChainSGClass<P_V,P_M>::updateAdaptedCovMatrix(
  const uqBaseVectorSequenceClass<P_V,P_M>& subChain,
  unsigned int                              idOfFirstPositionInSubChain,
  double&                                   lastChainSize,
  P_V&                                      lastMean,
  P_M&                                      lastAdaptedCovMatrix)
{
  double doubleSubChainSize = (double) subChain.sequenceSize();
  if (lastChainSize == 0) {
    UQ_FATAL_TEST_MACRO(subChain.sequenceSize() < 2,
                        m_env.rank(),
                        "uqMarkovChainSGClass<P_V,P_M>::updateAdaptedCovMatrix()",
                        "'subChain.sequenceSize()' should be >= 2");

    subChain.mean(0,subChain.sequenceSize(),lastMean);

    P_V tmpVec(m_vectorSpace.zeroVector());
    lastAdaptedCovMatrix = -doubleSubChainSize * matrixProduct(lastMean,lastMean);
    for (unsigned int i = 0; i < subChain.sequenceSize(); ++i) {
      subChain.getPositionValues(i,tmpVec);
      lastAdaptedCovMatrix += matrixProduct(tmpVec,tmpVec);
    }
    lastAdaptedCovMatrix /= (doubleSubChainSize - 1.); // That is why subChain size must be >= 2
  }
  else {
    UQ_FATAL_TEST_MACRO(subChain.sequenceSize() < 1,
                        m_env.rank(),
                        "uqMarkovChainSGClass<P_V,P_M>::updateAdaptedCovMatrix()",
                        "'subChain.sequenceSize()' should be >= 1");

    UQ_FATAL_TEST_MACRO(idOfFirstPositionInSubChain < 1,
                        m_env.rank(),
                        "uqMarkovChainSGClass<P_V,P_M>::updateAdaptedCovMatrix()",
                        "'idOfFirstPositionInSubChain' should be >= 1");

    P_V tmpVec (m_vectorSpace.zeroVector());
    P_V diffVec(m_vectorSpace.zeroVector());
    for (unsigned int i = 0; i < subChain.sequenceSize(); ++i) {
      double doubleCurrentId  = (double) (idOfFirstPositionInSubChain+i);
      subChain.getPositionValues(i,tmpVec);
      diffVec = tmpVec - lastMean;

      double ratio1         = (1. - 1./doubleCurrentId); // That is why idOfFirstPositionInSubChain must be >= 1
      double ratio2         = (1./(1.+doubleCurrentId));
      lastAdaptedCovMatrix  = ratio1 * lastAdaptedCovMatrix + ratio2 * matrixProduct(diffVec,diffVec);
      lastMean             += ratio2 * diffVec;
    } 
  }
  lastChainSize += doubleSubChainSize;

  return;
}
#endif // __UQ_MAC_SG2_H__
