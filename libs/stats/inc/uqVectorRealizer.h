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

#ifndef __UQ_REALIZER_H__
#define __UQ_REALIZER_H__

#include <uqVectorSequence.h>
#include <uqEnvironment.h>
#include <math.h>

//*****************************************************
// Classes to accomodate a probability density routine.
//*****************************************************

//*****************************************************
// Base class
//*****************************************************
template<class V, class M>
class uqBaseVectorRealizerClass {
public:
           uqBaseVectorRealizerClass(const char*                  prefix,
                                     const uqVectorSetClass<V,M>& unifiedImageSet,
                                     unsigned int                 subPeriod,
                                     const V&                     unifiedImageExpVector,
                                     const V&                     unifiedImageVarVector);
           uqBaseVectorRealizerClass(const char*                  prefix,
                                     const uqVectorSetClass<V,M>& unifiedImageSet,
                                     unsigned int                 subPeriod,
                                     const V&                     unifiedImageExpVector);
           uqBaseVectorRealizerClass(const char*                  prefix,
                                     const uqVectorSetClass<V,M>& unifiedImageSet,
                                     unsigned int                 subPeriod);

  virtual ~uqBaseVectorRealizerClass();

  const   uqVectorSetClass<V,M>& unifiedImageSet()              const;
          unsigned int           subPeriod      ()              const;
  virtual void                   realization    (V& nextValues) const = 0;

  const   V&                     unifiedImageExpVector()        const;
  const   V&                     unifiedImageVarVector()        const;

protected:
  const uqBaseEnvironmentClass& m_env;
        std::string             m_prefix;
  const uqVectorSetClass<V,M>&  m_unifiedImageSet;
        unsigned int            m_subPeriod;

        V*                      m_unifiedImageExpVector;
        V*                      m_unifiedImageVarVector;
};

template<class V, class M>
uqBaseVectorRealizerClass<V,M>::uqBaseVectorRealizerClass(
  const char*                  prefix,
  const uqVectorSetClass<V,M>& unifiedImageSet,
  unsigned int                 subPeriod,
  const V&                     unifiedImageExpVector,
  const V&                     unifiedImageVarVector)
  :
  m_env                  (unifiedImageSet.env()),
  m_prefix               ((std::string)(prefix)+"re_"),
  m_unifiedImageSet      (unifiedImageSet),
  m_subPeriod            (subPeriod),
  m_unifiedImageExpVector(new V(unifiedImageExpVector)),
  m_unifiedImageVarVector(new V(unifiedImageVarVector))
{
  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Entering uqBaseVectorRealizerClass<V,M>::constructor() [1]"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }

  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Leaving uqBaseVectorRealizerClass<V,M>::constructor() [1]"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }
}

template<class V, class M>
uqBaseVectorRealizerClass<V,M>::uqBaseVectorRealizerClass(
  const char*                  prefix,
  const uqVectorSetClass<V,M>& unifiedImageSet,
  unsigned int                 subPeriod,
  const V&                     unifiedImageExpVector)
  :
  m_env                  (unifiedImageSet.env()),
  m_prefix               ((std::string)(prefix)+"re_"),
  m_unifiedImageSet      (unifiedImageSet),
  m_subPeriod            (subPeriod),
  m_unifiedImageExpVector(new V(unifiedImageExpVector)),
  m_unifiedImageVarVector(unifiedImageSet.vectorSpace().newVector(INFINITY))
{
  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Entering uqBaseVectorRealizerClass<V,M>::constructor() [2]"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }

  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Leaving uqBaseVectorRealizerClass<V,M>::constructor() [2]"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }
}

template<class V, class M>
uqBaseVectorRealizerClass<V,M>::uqBaseVectorRealizerClass(
  const char*                  prefix,
  const uqVectorSetClass<V,M>& unifiedImageSet,
  unsigned int                 subPeriod)
  :
  m_env                  (unifiedImageSet.env()),
  m_prefix               ((std::string)(prefix)+"re_"),
  m_unifiedImageSet      (unifiedImageSet),
  m_subPeriod            (subPeriod),
  m_unifiedImageExpVector(unifiedImageSet.vectorSpace().newVector(       0.)),
  m_unifiedImageVarVector(unifiedImageSet.vectorSpace().newVector( INFINITY))
{
  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Entering uqBaseVectorRealizerClass<V,M>::constructor() [4]"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }

  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Leaving uqBaseVectorRealizerClass<V,M>::constructor() [4]"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }
}

template<class V, class M>
uqBaseVectorRealizerClass<V,M>::~uqBaseVectorRealizerClass()
{
}

template<class V, class M>
unsigned int
uqBaseVectorRealizerClass<V,M>::subPeriod() const
{
  return m_subPeriod;
}

template <class V, class M>
const V&
uqBaseVectorRealizerClass<V,M>::unifiedImageExpVector() const
{
  return *m_unifiedImageExpVector;
}

template <class V, class M>
const V&
uqBaseVectorRealizerClass<V,M>::unifiedImageVarVector() const
{
  return *m_unifiedImageVarVector;
}

template<class V, class M>
const uqVectorSetClass<V,M>&
uqBaseVectorRealizerClass<V,M>::unifiedImageSet() const
{
  return m_unifiedImageSet;
}

//*****************************************************
// Generic class
//*****************************************************
template<class V, class M>
class uqGenericVectorRealizerClass : public uqBaseVectorRealizerClass<V,M> {
public:
  uqGenericVectorRealizerClass(const char*                  prefix,
                               const uqVectorSetClass<V,M>& unifiedImageSet,
                               unsigned int                 subPeriod,
                               double (*routinePtr)(const void* routineDataPtr, V& nextParamValues),
                               const void* routineDataPtr);
 ~uqGenericVectorRealizerClass();

  void realization(V& nextValues) const;

private:
  double (*m_routinePtr)(const void* routineDataPtr, V& nextParamValues);
  const void* m_routineDataPtr;

  using uqBaseVectorRealizerClass<V,M>::m_env;
  using uqBaseVectorRealizerClass<V,M>::m_prefix;
  using uqBaseVectorRealizerClass<V,M>::m_unifiedImageSet;
  using uqBaseVectorRealizerClass<V,M>::m_subPeriod;
};

template<class V, class M>
uqGenericVectorRealizerClass<V,M>::uqGenericVectorRealizerClass(
  const char*                  prefix,
  const uqVectorSetClass<V,M>& unifiedImageSet,
  unsigned int                 subPeriod,
  double (*routinePtr)(const void* routineDataPtr, V& nextParamValues),
  const void* routineDataPtr)
  :
  uqBaseVectorRealizerClass<V,M>(((std::string)(prefix)+"gen").c_str(),unifiedImageSet,subPeriod),
  m_routinePtr    (routinePtr),
  m_routineDataPtr(routineDataPtr)
{
  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Entering uqGenericVectorRealizerClass<V,M>::constructor()"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }

  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Leaving uqGenericVectorRealizerClass<V,M>::constructor()"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }
}

template<class V, class M>
uqGenericVectorRealizerClass<V,M>::~uqGenericVectorRealizerClass()
{
}

template<class V, class M>
void
uqGenericVectorRealizerClass<V,M>::realization(V& nextValues) const
{
  m_routinePtr(m_routineDataPtr,nextValues);
  return;
}

//*****************************************************
// Gaussian class
//*****************************************************
template<class V, class M>
class uqGaussianVectorRealizerClass : public uqBaseVectorRealizerClass<V,M> {
public:
  uqGaussianVectorRealizerClass(const char* prefix,
				const uqVectorSetClass<V,M>& unifiedImageSet,
				const V& expVector, // vector of mean values
				const M& lowerCholCovMatrix); // lower triangular matrix resulting from Cholesky decomposition of the covariance matrix

  ~uqGaussianVectorRealizerClass();

  void realization             (V& nextValues) const;
  void updateExpVector         (const V& newExpVector);
  void updateLowerCholCovMatrix(const M& newLowerCholCovMatrix);
    
private:
  M* m_lowerCholCovMatrix;

  using uqBaseVectorRealizerClass<V,M>::m_env;
  using uqBaseVectorRealizerClass<V,M>::m_prefix;
  using uqBaseVectorRealizerClass<V,M>::m_unifiedImageSet;
  using uqBaseVectorRealizerClass<V,M>::m_subPeriod;
  using uqBaseVectorRealizerClass<V,M>::m_unifiedImageExpVector;
};

template<class V, class M>
uqGaussianVectorRealizerClass<V,M>::uqGaussianVectorRealizerClass(const char* prefix,
								  const uqVectorSetClass<V,M>& unifiedImageSet,
								  const V& expVector,
								  const M& lowerCholCovMatrix)
  :
  uqBaseVectorRealizerClass<V,M>( ((std::string)(prefix)+"gau").c_str(), unifiedImageSet, 0 ),
  m_lowerCholCovMatrix(new M(lowerCholCovMatrix))
{
  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Entering uqGaussianVectorRealizerClass<V,M>::constructor()"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }

  *m_unifiedImageExpVector = expVector;

  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Leaving uqGaussianVectorRealizerClass<V,M>::constructor()"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }
}
								  
template<class V, class M>
uqGaussianVectorRealizerClass<V,M>::~uqGaussianVectorRealizerClass()
{
  delete m_lowerCholCovMatrix;
}

template<class V, class M>
void
uqGaussianVectorRealizerClass<V,M>::realization(V& nextValues) const
{
  V iidGaussianVector(m_unifiedImageSet.vectorSpace().zeroVector());
  iidGaussianVector.cwSetGaussian(m_env.rng(), 0.0, 1.0);

  nextValues = (*m_unifiedImageExpVector) + (*m_lowerCholCovMatrix)*iidGaussianVector;
  return;
}

template<class V, class M>
void
uqGaussianVectorRealizerClass<V,M>::updateExpVector(const V& newExpVector)
{
  // delete old expected values (alloced at construction or last call to this function)
  delete m_unifiedImageExpVector;
  m_unifiedImageExpVector = new V(newExpVector);
  return;
}

template<class V, class M>
void
uqGaussianVectorRealizerClass<V,M>::updateLowerCholCovMatrix(const M& newLowerCholCovMatrix)
{
  // delete old expected values (alloced at construction or last call to this function)
  delete m_lowerCholCovMatrix;
  m_lowerCholCovMatrix = new M(newLowerCholCovMatrix);
  return;
}

//*****************************************************
// Sequential class
//*****************************************************
template<class V, class M>
class uqSequentialVectorRealizerClass : public uqBaseVectorRealizerClass<V,M> {
public:
  uqSequentialVectorRealizerClass(const char*                           prefix,
                                  const uqBaseVectorSequenceClass<V,M>& chain);
 ~uqSequentialVectorRealizerClass();

  void realization(V& nextValues) const;

private:
  const uqBaseVectorSequenceClass<V,M>& m_chain;
  mutable unsigned int                  m_currentChainPos;

  using uqBaseVectorRealizerClass<V,M>::m_env;
  using uqBaseVectorRealizerClass<V,M>::m_prefix;
  using uqBaseVectorRealizerClass<V,M>::m_unifiedImageSet;
  using uqBaseVectorRealizerClass<V,M>::m_subPeriod;
};

template<class V, class M>
uqSequentialVectorRealizerClass<V,M>::uqSequentialVectorRealizerClass(
  const char*                           prefix,
  const uqBaseVectorSequenceClass<V,M>& chain)
  :
  uqBaseVectorRealizerClass<V,M>(((std::string)(prefix)+"seq").c_str(),chain.unifiedValuesBox(),chain.subSequenceSize(),chain.unifiedMeanValues(),chain.unifiedSampleVarianceValues()), // IMPORTANT
  m_chain          (chain),
  m_currentChainPos(0)
{
  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 0)) {
    *m_env.subScreenFile() << "In uqSequentialVectorRealizerClass<V,M>::constructor()"
                           << ": m_chain.subSequenceSize() = " << m_chain.subSequenceSize()
                           << std::endl;
  }
}

template<class V, class M>
uqSequentialVectorRealizerClass<V,M>::~uqSequentialVectorRealizerClass()
{
}

template<class V, class M>
void
uqSequentialVectorRealizerClass<V,M>::realization(V& nextParamValues) const
{
  m_chain.getPositionValues(m_currentChainPos++,nextParamValues);
  if (m_currentChainPos >= m_subPeriod) m_currentChainPos = 0;

  return;
}

//*****************************************************
// Uniform class
//*****************************************************
template<class V, class M>
class uqUniformVectorRealizerClass : public uqBaseVectorRealizerClass<V,M> {
public:
  uqUniformVectorRealizerClass(const char*                  prefix,
                               const uqVectorSetClass<V,M>& unifiedImageSet);
 ~uqUniformVectorRealizerClass();

  void realization(V& nextValues) const;

private:
  using uqBaseVectorRealizerClass<V,M>::m_env;
  using uqBaseVectorRealizerClass<V,M>::m_prefix;
  using uqBaseVectorRealizerClass<V,M>::m_unifiedImageSet;
  using uqBaseVectorRealizerClass<V,M>::m_subPeriod;
};

template<class V, class M>
uqUniformVectorRealizerClass<V,M>::uqUniformVectorRealizerClass(
  const char*                  prefix,
  const uqVectorSetClass<V,M>& unifiedImageSet)
  :
  uqBaseVectorRealizerClass<V,M>(((std::string)(prefix)+"gen").c_str(),unifiedImageSet,std::numeric_limits<unsigned int>::max())
{
  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Entering uqUniformVectorRealizerClass<V,M>::constructor()"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }

  if ((m_env.subScreenFile()) && (m_env.verbosity() >= 5)) {
    *m_env.subScreenFile() << "Leaving uqUniformVectorRealizerClass<V,M>::constructor()"
                           << ": prefix = " << m_prefix
                           << std::endl;
  }
}

template<class V, class M>
uqUniformVectorRealizerClass<V,M>::~uqUniformVectorRealizerClass()
{
}

template<class V, class M>
void
uqUniformVectorRealizerClass<V,M>::realization(V& nextValues) const
{
  const uqBoxSubsetClass<V,M>* imageBox = dynamic_cast<const uqBoxSubsetClass<V,M>* >(&m_unifiedImageSet);

  UQ_FATAL_TEST_MACRO(imageBox == NULL,
                      m_env.rank(),
                      "uqUniformVectorRealizerClass<V,M>::realization()",
                      "only box images are supported right now");
  
  nextValues.cwSetUniform(m_env.rng(),imageBox->minValues(),imageBox->maxValues());
  return;
}
#endif // __UQ_REALIZER_H__
