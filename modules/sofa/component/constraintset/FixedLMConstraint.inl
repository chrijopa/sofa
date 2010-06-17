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
#ifndef SOFA_COMPONENT_CONSTRAINTSET_FIXEDLMCONSTRAINT_INL
#define SOFA_COMPONENT_CONSTRAINTSET_FIXEDLMCONSTRAINT_INL

#include <sofa/component/constraintset/FixedLMConstraint.h>
#include <sofa/simulation/common/Simulation.h>
#include <sofa/helper/gl/template.h>





namespace sofa
{

namespace component
{

namespace constraintset
{

using namespace sofa::helper;


// Define TestNewPointFunction
template< class DataTypes>
bool FixedLMConstraint<DataTypes>::FCTestNewPointFunction(int /*nbPoints*/, void* param, const sofa::helper::vector< unsigned int > &, const sofa::helper::vector< double >& )
{
    FixedLMConstraint<DataTypes> *fc= (FixedLMConstraint<DataTypes> *)param;
    if (fc)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Define RemovalFunction
template< class DataTypes>
void FixedLMConstraint<DataTypes>::FCRemovalFunction(int pointIndex, void* param)
{
    FixedLMConstraint<DataTypes> *fc= (FixedLMConstraint<DataTypes> *)param;
    if (fc)
    {
        fc->removeConstraint((unsigned int) pointIndex);
    }
    return;
}

template <class DataTypes>
void FixedLMConstraint<DataTypes>::clearConstraints()
{
    f_indices.beginEdit()->clear();
    f_indices.endEdit();
}

template <class DataTypes>
void FixedLMConstraint<DataTypes>::addConstraint(unsigned int index)
{
    f_indices.beginEdit()->push_back(index);
    f_indices.endEdit();
}

template <class DataTypes>
void FixedLMConstraint<DataTypes>::removeConstraint(unsigned int index)
{
    removeValue(*f_indices.beginEdit(),index);
    f_indices.endEdit();
}


template <class DataTypes>
void FixedLMConstraint<DataTypes>::initFixedPosition()
{
    this->restPosition.clear();
    const VecCoord& x = *this->constrainedObject1->getX();
    const SetIndexArray & indices = this->f_indices.getValue().getArray();
    for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it)
    {
        unsigned int index=*it;
        this->restPosition.insert(std::make_pair(index, x[index]));
    }
}

template <class DataTypes>
void FixedLMConstraint<DataTypes>::init()
{
    core::behavior::LMConstraint<DataTypes,DataTypes>::init();

    topology = this->getContext()->getMeshTopology();

    // Initialize functions and parameters
    topology::PointSubset my_subset = f_indices.getValue();

    my_subset.setTestFunction(FCTestNewPointFunction);
    my_subset.setRemovalFunction(FCRemovalFunction);

    my_subset.setTestParameter( (void *) this );
    my_subset.setRemovalParameter( (void *) this );


    X[0]=1; X[1]=0; X[2]=0;
    Y[0]=0; Y[1]=1; Y[2]=0;
    Z[0]=0; Z[1]=0; Z[2]=1;

    initFixedPosition();

}

// Handle topological changes
template <class DataTypes> void FixedLMConstraint<DataTypes>::handleTopologyChange()
{
    std::list<const TopologyChange *>::const_iterator itBegin=topology->firstChange();
    std::list<const TopologyChange *>::const_iterator itEnd =topology->lastChange();

    f_indices.beginEdit()->handleTopologyEvents(itBegin,itEnd,this->constrainedObject1->getSize());
}


template<class DataTypes>
void FixedLMConstraint<DataTypes>::buildConstraintMatrix(unsigned int &constraintId, core::VecId /*position*/)
{
    idxX.clear();
    idxY.clear();
    idxZ.clear();
    const SetIndexArray &indices = f_indices.getValue().getArray();

    for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it)
    {
        const unsigned int index=*it;

        //Constraint degree of freedom along X direction

        SparseVecDeriv VX; VX.add(index,X);
        registerEquationInJ1(constraintId, VX);
        idxX.push_back(constraintId++);

        //Constraint degree of freedom along X direction
        SparseVecDeriv VY; VY.add(index,Y);
        registerEquationInJ1(constraintId, VY);
        idxY.push_back(constraintId++);

        //Constraint degree of freedom along Z direction
        SparseVecDeriv VZ; VZ.add(index,Z);
        registerEquationInJ1(constraintId, VZ);
        idxZ.push_back(constraintId++);

        this->constrainedObject1->forceMask.insertEntry(index);
    }
}


template<class DataTypes>
void FixedLMConstraint<DataTypes>::writeConstraintEquations(VecId id, ConstOrder Order)
{

    typedef core::behavior::BaseMechanicalState::VecId VecId;
    const SetIndexArray & indices = f_indices.getValue().getArray();

    unsigned int counter=0;
    for (SetIndexArray::const_iterator it = indices.begin(); it != indices.end(); ++it,++counter)
    {
        const unsigned int index = *it;

        core::behavior::BaseLMConstraint::ConstraintGroup *constraint = this->addGroupConstraint(Order);
        SReal correctionX=0,correctionY=0,correctionZ=0;
        switch(Order)
        {
        case core::behavior::BaseLMConstraint::ACC :
        case core::behavior::BaseLMConstraint::VEL :
        {
            correctionX = this->constrainedObject1->getConstraintJacobianTimesVecDeriv(idxX[counter],id);
            correctionY = this->constrainedObject1->getConstraintJacobianTimesVecDeriv(idxY[counter],id);
            correctionZ = this->constrainedObject1->getConstraintJacobianTimesVecDeriv(idxZ[counter],id);
            break;
        }
        case core::behavior::BaseLMConstraint::POS :
        {
            const VecCoord &x =*(this->constrainedObject1->getVecCoord(id.index));

            //If a new particle has to be fixed, we add its current position as rest position
            if (restPosition.find(index) == this->restPosition.end())
            {
                restPosition.insert(std::make_pair(index, x[index]));
            }


            Coord v=x[index]-restPosition[index];
            correctionX=v[0];
            correctionY=v[1];
            correctionZ=v[2];
            break;
        }
        };

        constraint->addConstraint( idxX[counter], -correctionX);
        constraint->addConstraint( idxY[counter], -correctionY);
        constraint->addConstraint( idxZ[counter], -correctionZ);

    }
}



template <class DataTypes>
void FixedLMConstraint<DataTypes>::draw()
{
    if (!this->getContext()->getShowBehaviorModels()) return;
    const VecCoord& x = *this->constrainedObject1->getX();
    //serr<<"FixedLMConstraint<DataTypes>::draw(), x.size() = "<<x.size()<<sendl;




    const SetIndexArray & indices = f_indices.getValue().getArray();

    std::vector< Vector3 > points;
    Vector3 point;
    unsigned int sizePoints= (Coord::static_size <=3)?Coord::static_size:3;
    //serr<<"FixedLMConstraint<DataTypes>::draw(), indices = "<<indices<<sendl;
    for (SetIndexArray::const_iterator it = indices.begin();
            it != indices.end();
            ++it)
    {
        for (unsigned int s=0; s<sizePoints; ++s) point[s] = x[*it][s];
        points.push_back(point);
    }
    if( _drawSize.getValue() == 0) // old classical drawing by points
    {
        simulation::getSimulation()->DrawUtility.drawPoints(points, 10, Vec<4,float>(1,0.5,0.5,1));
    }
    else
    {
        simulation::getSimulation()->DrawUtility.drawSpheres(points, (float)_drawSize.getValue(), Vec<4,float>(1.0f,0.35f,0.35f,1.0f));
    }
}

// Specialization for rigids
#ifndef SOFA_FLOAT
template <>
void FixedLMConstraint<Rigid3dTypes >::draw();
#endif
#ifndef SOFA_DOUBLE
template <>
void FixedLMConstraint<Rigid3fTypes >::draw();
#endif





} // namespace constraintset

} // namespace component

} // namespace sofa

#endif


