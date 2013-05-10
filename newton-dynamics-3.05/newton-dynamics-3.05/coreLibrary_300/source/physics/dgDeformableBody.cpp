/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "dgPhysicsStdafx.h"

#if 0
#include "dgWorld.h"
#include "dgDeformableBody.h"
#include "dgCollisionInstance.h"
#include "dgCollisionDeformableMesh.h"


dgDeformableBody::dgDeformableBody()
	:dgBody()
{
	m_type = m_deformableBody;
	m_rtti |= m_deformableBodyRTTI;
	m_isDeformable = true;
}

dgDeformableBody::dgDeformableBody(dgWorld* const world, const dgTree<const dgCollision*, dgInt32>* const collisionCashe, OnBodyDeserialize bodyCallback, dgDeserialize serializeCallback, void* const userData)
	:dgBody(world, collisionCashe, bodyCallback, serializeCallback, userData)
{
	m_type = m_deformableBody;
	m_rtti |= m_deformableBodyRTTI;
	_ASSERTE (0);
}

dgDeformableBody::~dgDeformableBody()
{
}


void dgDeformableBody::Serialize (const dgTree<dgInt32, const dgCollision*>* const collisionCashe, OnBodySerialize bodyCallback, dgSerialize serializeCallback, void* const userData)
{
	dgBody::Serialize (collisionCashe, bodyCallback, serializeCallback, userData);
	_ASSERTE (0);
}

void dgDeformableBody::SetMassMatrix (dgFloat32 mass, dgFloat32 Ix, dgFloat32 Iy, dgFloat32 Iz)
{
	dgBody::SetMassMatrix (mass, Ix, Iy, Iz);
	_ASSERTE (m_collision->IsType(dgCollision::dgCollisionDeformableMesh_RTTI));

	dgCollisionDeformableMesh* const deformableCollision = (dgCollisionDeformableMesh*) m_collision;
	deformableCollision->SetParticlesMasses (mass);
}

void dgDeformableBody::ApplyExtenalForces (dgFloat32 timestep, dgInt32 threadIndex)
{
	m_dynamicsLru =  m_world->m_dynamicsLru + DG_BODY_LRU_STEP;

	if (m_collision->IsType(dgCollision::dgCollisionDeformableMesh_RTTI)) {
		dgBody::ApplyExtenalForces (timestep, threadIndex);
		_ASSERTE (m_collision->IsType(dgCollision::dgCollisionDeformableMesh_RTTI));

		dgCollisionDeformableMesh* const deformableCollision = (dgCollisionDeformableMesh*) m_collision;
		deformableCollision->ApplyExternalAndInternalForces (this, timestep, threadIndex);
	}
}

void dgDeformableBody::SetVelocity (const dgVector& velocity)
{
	dgBody::SetVelocity(velocity);
	_ASSERTE (m_collision->IsType(dgCollision::dgCollisionDeformableMesh_RTTI));

	dgCollisionDeformableMesh* const deformableCollision = (dgCollisionDeformableMesh*) m_collision;
	deformableCollision->SetParticlesVelocities (velocity);
}


bool dgDeformableBody::IsInEquilibrium  () const
{
	// for now soft bodies do not rest
	return false;
}

void dgDeformableBody::SetMatrix(const dgMatrix& matrix)
{
	if (m_collision->IsType(dgCollision::dgCollisionDeformableMesh_RTTI)) {
		dgCollisionDeformableMesh* const deformableCollision = (dgCollisionDeformableMesh*) m_collision;
		dgMatrix indentityRotation (matrix);
		indentityRotation.m_posit = matrix.m_posit;

		deformableCollision->SetMatrix(matrix);
		dgBody::SetMatrix(indentityRotation);
		
	}
}

#endif