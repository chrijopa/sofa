/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 4      *
*                (c) 2006-2009 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_COMPONENT_ENGINE_RANDOMPOINTDISTRIBUTIONINSURFACE_H
#define SOFA_COMPONENT_ENGINE_RANDOMPOINTDISTRIBUTIONINSURFACE_H

#if !defined(__GNUC__) || (__GNUC__ > 3 || (_GNUC__ == 3 && __GNUC_MINOR__ > 3))
#pragma once
#endif

#include <sofa/defaulttype/Vec.h>
#include <sofa/core/DataEngine.h>
#include <sofa/core/objectmodel/BaseObject.h>
#include <sofa/core/behavior/MechanicalState.h>
#include <sofa/core/topology/BaseMeshTopology.h>
#include <sofa/component/topology/PointSubset.h>
#include <sofa/defaulttype/Vec3Types.h>
#include <sofa/component/collision/TriangleOctree.h>
#include <sofa/helper/RandomGenerator.h>

namespace sofa
{

namespace component
{

namespace engine
{

using namespace core::behavior;
using namespace core::topology;
using namespace core::objectmodel;

/**
 * This class extrudes a surface
 */
template <class DataTypes>
class RandomPointDistributionInSurface : public core::DataEngine
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(RandomPointDistributionInSurface,DataTypes),core::DataEngine);
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename DataTypes::Real Real;
    typedef defaulttype::Vec<3,Real> Vec3;

public:

    RandomPointDistributionInSurface();

    ~RandomPointDistributionInSurface() {}

    void init();

    void reinit();

    void update();

    void draw(const core::visual::VisualParams* vparams);

    virtual std::string getTemplateName() const
    {
        return templateName(this);
    }

    static std::string templateName(const RandomPointDistributionInSurface<DataTypes>* = NULL)
    {
        return DataTypes::Name();
    }
    bool initialized;
    helper::RandomGenerator rg;
    Data<unsigned int> randomSeed;
    Data<bool> isVisible;
    Data<bool> drawOutputPoints;
    Data<Real> minDistanceBetweenPoints;
    Data<unsigned int> numberOfInPoints;
    Data<unsigned int> numberOfTests;

    Data<VecCoord> f_vertices;
    Data< helper::vector<BaseMeshTopology::Triangle> > f_triangles;
    Data<VecCoord> f_inPoints;
    Data<VecCoord> f_outPoints;

    unsigned int safeCounter;
    unsigned int safeLimit;

protected:
    VecCoord directions;
    collision::TriangleOctreeRoot trianglesOctree;
    void getBBox(Vec3 &minBBox, Vec3 &maxBBox);
    void generateRandomDirections();
    Vec3 generateRandomPoint(const Vec3 &minBBox, const Vec3 &maxBBox);
    bool isInside(Coord p);
    bool testDistance(Coord p);
};

#if defined(WIN32) && !defined(SOFA_COMPONENT_ENGINE_RANDOMPOINTDISTRIBUTIONINSURFACE_CPP)
#pragma warning(disable : 4231)
#ifndef SOFA_FLOAT
template class SOFA_COMPONENT_ENGINE_API RandomPointDistributionInSurface<defaulttype::Vec3dTypes>;
#endif //SOFA_FLOAT
#ifndef SOFA_DOUBLE
template class SOFA_COMPONENT_ENGINE_API RandomPointDistributionInSurface<defaulttype::Vec3fTypes>;
#endif //SOFA_DOUBLE
#endif

} // namespace engine

} // namespace component

} // namespace sofa

#endif //SOFA_COMPONENT_ENGINE_RANDOMPOINTDISTRIBUTIONINSURFACE_H
