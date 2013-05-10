/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include <toolbox_stdafx.h>
#include "TargaToOpenGl.h"
#include "../DemoMesh.h"
#include "../DemoEntityManager.h"
#include "../DemoCamera.h"
#include "PhysicsUtils.h"
#include "DebugDisplay.h"
#include "HeightFieldPrimitive.h"

#define GAUSSIAN_BELL 2
//#define GAUSSIAN_BELL	1
#define TILE_SIZE	128

// uncomment this to remove the nasty crease generated by mid point displacement
#define RE_SAMPLE_CORNER

//class HeightfieldMap: public DemoMesh
class HeightfieldMap
{
	public:
	
	static dFloat Guassian (dFloat freq)
	{
		dFloat r = 0.0f;
		int maxCount = 2 * GAUSSIAN_BELL + 1;
		for (int i = 0; i < maxCount; i ++) {
			dFloat t = 2.0f * dFloat (dRand()) / dFloat(dRAND_MAX) - 1.0f;
			r += t;
		}
freq *= 0.5f;
		return freq * r / maxCount;
	}

/*
	static void SetBaseHeight (dFloat* const elevation, int size)
	{
		dFloat minhigh = 1.0e10;
		for (int i = 0; i < size * size; i ++) {
			if (elevation[i] < minhigh) {
				minhigh = elevation[i];
			}
		}
		for (int i = 0; i < size * size; i ++) {
			elevation[i] -= minhigh;
		}
	}

	static void MakeCalderas (dFloat* const elevation, int size, dFloat baseHigh, dFloat roughness)
	{
		for (int i = 0; i < size * size; i ++) {
			//dFloat h =  baseHigh + Guassian (roughness);
			dFloat h =  baseHigh;
			if (elevation[i] > h) {
				elevation[i] = h - (elevation[i] - h);
			}
		}
	}
*/

	static void MakeMap (dFloat* const elevation, dFloat** const map, int size)
	{
		for (int i = 0; i < size; i ++) {
			map[i] = &elevation[i * size];
		}
	}

	static void MakeMap (dVector* const normal, dVector** const map, int size)
	{
		for (int i = 0; i < size; i ++) {
			map[i] = &normal[i * size];
		}
	}


/*
	static DemoMesh* CreateVisualMesh (dFloat* const elevation, int size, dFloat cellSize, dFloat texelsDensity)
	{
		dVector* const normals = new dVector [size * size];

		float* elevationMap[4096];
		dVector* normalMap[4096];

		MakeMap (elevation, elevationMap, size);
		MakeMap (normals, normalMap, size);

		memset (normals, 0, (size * size) * sizeof (dVector));
		for (int z = 0; z < size - 1; z ++) {
			for (int x = 0; x < size - 1; x ++) {
				dVector p0 ((x + 0) * cellSize, elevationMap[z + 0][x + 0], (z + 0) * cellSize);
				dVector p1 ((x + 1) * cellSize, elevationMap[z + 0][x + 1], (z + 0) * cellSize);
				dVector p2 ((x + 1) * cellSize, elevationMap[z + 1][x + 1], (z + 1) * cellSize);
				dVector p3 ((x + 0) * cellSize, elevationMap[z + 1][x + 0], (z + 1) * cellSize);

				dVector e10 (p1 - p0);
				dVector e20 (p2 - p0);
				dVector n0 (e20 * e10);
				n0 = n0.Scale ( 1.0f / dSqrt (n0 % n0));
				normalMap [z + 0][x + 0] += n0;
				normalMap [z + 0][x + 1] += n0;
				normalMap [z + 1][x + 1] += n0;

				dVector e30 (p3 - p0);
				dVector n1 (e30 * e20);
				n1 = n1.Scale ( 1.0f / dSqrt (n1 % n1));
				normalMap [z + 0][x + 0] += n1;
				normalMap [z + 1][x + 0] += n1;
				normalMap [z + 1][x + 1] += n1;
			}
		}

		for (int i = 0; i < size * size; i ++) {
			normals[i] = normals[i].Scale (1.0f / sqrtf (normals[i] % normals[i]));
		}


		HeightfieldMap* const mesh = new HeightfieldMap () ;
		mesh->AllocVertexData (size * size);

		dFloat* const vertex = mesh->m_vertex;
		dFloat* const normal = mesh->m_normal;
		dFloat* const uv = mesh->m_uv;

		int index = 0;
		for (int z = 0; z < size; z ++) {
			for (int x = 0; x < size; x ++) {
				vertex[index * 3 + 0] = x * cellSize;
				vertex[index * 3 + 1] = elevationMap[z][x];
				vertex[index * 3 + 2] = z * cellSize;

				normal[index * 3 + 0] = normalMap[z][x].m_x;
				normal[index * 3 + 1] = normalMap[z][x].m_y;
				normal[index * 3 + 2] = normalMap[z][x].m_z;

				uv[index * 2 + 0] = x * texelsDensity;
				uv[index * 2 + 1] = z * texelsDensity;
				index ++;
			}
		}


		int segmentsCount = (size - 1) / TILE_SIZE;
		for (int z0 = 0; z0 < segmentsCount; z0 ++) {
			int z = z0 * TILE_SIZE;
			for (int x0 = 0; x0 < segmentsCount; x0 ++ ) {
				int x = x0 * TILE_SIZE;

				DemoSubMesh* const tile = mesh->AddSubMesh();
				tile->AllocIndexData (TILE_SIZE * TILE_SIZE * 6);
				unsigned* const indexes = tile->m_indexes;

				strcpy (tile->m_textureName, "grassanddirt.tga");
				tile->m_textureHandle = LoadTexture(tile->m_textureName);

				int index = 0;
				int x1 = x + TILE_SIZE;
				int z1 = z + TILE_SIZE;
				for (int z0 = z; z0 < z1; z0 ++) {
					for (int x0 = x; x0 < x1; x0 ++) {
						int i0 = x0 + 0 + (z0 + 0) * size;
						int i1 = x0 + 1 + (z0 + 0) * size;
						int i2 = x0 + 1 + (z0 + 1) * size;
						int i3 = x0 + 0 + (z0 + 1) * size;

						indexes[index + 0] = i0;
						indexes[index + 1] = i2;
						indexes[index + 2] = i1;

						indexes[index + 3] = i0;
						indexes[index + 4] = i3;
						indexes[index + 5] = i2;
						index += 6;
					}
				}
				index*=1;
			}
		}

		delete[] normals;
		return mesh;
	}
*/

	static void ApplySmoothFilter (dFloat* const elevation, int size)
	{
		float* map0[4096 + 1];
		float* map1[4096 + 1];
		float* const buffer = new dFloat [size * size];

		MakeMap (elevation, map0, size);
		MakeMap (buffer, map1, size);

		for (int z = 0; z < size; z ++) {
			map1[z][0] = map0[z][0];
			map1[z][size - 1] = map0[z][size - 1];
			for (int x = 1; x < (size - 1); x ++) {
				map1[z][x] = map0[z][x-1] * 0.25f +  map0[z][x] * 0.5f + map0[z][x+1] * 0.25f; 
			}
		}

		for (int x = 0; x < size; x ++) {
			map0[0][x] = map1[0][x];
			map0[size - 1][x] = map1[size - 1][x];
			for (int z = 1; z < (size - 1); z ++) {
				map0[z][x] = map1[z-1][x] * 0.25f +  map1[z][x] * 0.5f + map1[z+1][x] * 0.25f; 
			}
		}

		delete[] buffer;
	}


	static dFloat GetElevation (dFloat size, dFloat elevation, dFloat maxH, dFloat minH, dFloat roughness)
	{
		dFloat h = pow (dFloat (size) * elevation, 1.0f + roughness);
		if (h > maxH) {
			h = maxH;	
		} else if (h < minH){
			h = minH;
		}
		return h;
	}

	static void MakeFractalTerrain (dFloat* const elevation, int sizeInPowerOfTwos, dFloat elevationScale, dFloat roughness, dFloat maxElevation, dFloat minElevation)
	{
		float* map[4096 + 1];
		int size = (1 << sizeInPowerOfTwos) + 1;
		_ASSERTE (size < sizeof (map) / sizeof map[0]);
		MakeMap (elevation, map, size);

		dFloat f = GetElevation (size, elevationScale, maxElevation, minElevation, roughness);
		map[0][0] = Guassian(f);
		map[0][size-1] = Guassian(f);
		map[size-1][0] = Guassian(f);
		map[size-1][size-1] = Guassian(f);
		for (int frequency = size - 1; frequency > 1; frequency = frequency / 2 ) {
			//dFloat f = pow (dFloat (frequency) * elevationScale, 1.0f + roughness);
			dFloat f = GetElevation (frequency, elevationScale, maxElevation, minElevation, roughness);

			for(int y0 = 0; y0 < (size - frequency); y0 += frequency) {
				int y1 = y0 + frequency / 2;
				int y2 = y0 + frequency;

				for(int x0 = 0; x0 < (size - frequency); x0 += frequency) {
					int x1 = x0 + frequency / 2;
					int x2 = x0 + frequency;

					map[y1][x1] = (map[y0][x0] + map[y0][x2] + map[y2][x0] + map[y2][x2]) * 0.25f + Guassian(f);

					map[y0][x1] = (map[y0][x0] + map[y0][x2]) * 0.5f + Guassian(f);
					map[y2][x1] = (map[y2][x0] + map[y2][x2]) * 0.5f + Guassian(f);

					map[y1][x0] = (map[y0][x0] + map[y2][x0]) * 0.5f + Guassian(f);
					map[y1][x2] = (map[y0][x2] + map[y2][x2]) * 0.5f + Guassian(f);

					// this trick eliminate the creases 
					#ifdef RE_SAMPLE_CORNER
						map[y0][x0] = (map[y0][x1] + map[y1][x0]) * 0.5f + Guassian(f);
						map[y0][x2] = (map[y0][x1] + map[y1][x2]) * 0.5f + Guassian(f);
						map[y2][x0] = (map[y1][x0] + map[y2][x1]) * 0.5f + Guassian(f);
						map[y2][x2] = (map[y2][x1] + map[y1][x2]) * 0.5f + Guassian(f);

//						map[y0][x1] = (map[y0][x0] + map[y0][x2]) * 0.5f + Guassian(f);
//						map[y2][x1] = (map[y2][x0] + map[y2][x2]) * 0.5f + Guassian(f);
//						map[y1][x0] = (map[y0][x0] + map[y2][x0]) * 0.5f + Guassian(f);
//						map[y1][x2] = (map[y0][x2] + map[y2][x2]) * 0.5f + Guassian(f);
					#endif
				}
			}
		}
	}

	static NewtonBody* CreateHeightFieldTerrain (DemoEntityManager* const scene, int sizeInPowerOfTwos, dFloat cellSize, dFloat elevationScale, dFloat roughness, dFloat maxElevation, dFloat minElevation)
	{
		int size = (1 << sizeInPowerOfTwos) + 1 ;
		dFloat* const elevation = new dFloat [size * size];
		//MakeFractalTerrain (elevation, sizeInPowerOfTwos, elevationScale, roughness, maxElevation, minElevation);
		MakeFractalTerrain (elevation, sizeInPowerOfTwos, elevationScale, roughness, maxElevation, minElevation);

		for (int i = 0; i < 4; i ++) {
			ApplySmoothFilter (elevation, size);
		}

		//		SetBaseHeight (elevation, size);
		// apply simple calderas
		//		MakeCalderas (elevation, size, maxElevation * 0.7f, maxElevation * 0.1f);

		//	// create the visual mesh
		DemoMesh* const mesh = new DemoMesh ("terrain", elevation, size, cellSize, 1.0f/16.0f, TILE_SIZE);

		DemoEntity* const entity = new DemoEntity(GetIdentityMatrix(), NULL);
		scene->Append (entity);
		entity->SetMesh(mesh);
		mesh->Release();

		// create the height field collision and rigid body

		// create the attribute map
		int width = size;
		int height = size;
		char* const attibutes = new char [size * size];
		memset (attibutes, 0, width * height * sizeof (char));
		NewtonCollision* collision = NewtonCreateHeightFieldCollision (scene->GetNewton(), width, height, 1, elevation, attibutes, cellSize, 0);


#ifdef USE_STATIC_MESHES_DEBUG_COLLISION
		NewtonStaticCollisionSetDebugCallback (collision, ShowMeshCollidingFaces);
#endif

		NewtonCollisionInfoRecord collisionInfo;
		// keep the compiler happy
		memset (&collisionInfo, 0, sizeof (NewtonCollisionInfoRecord));
		NewtonCollisionGetInfo (collision, &collisionInfo);

		width = collisionInfo.m_heightField.m_width;
		height = collisionInfo.m_heightField.m_height;
		//elevations = collisionInfo.m_heightField.m_elevation;

		dVector boxP0; 
		dVector boxP1; 
		// get the position of the aabb of this geometry
		dMatrix matrix (entity->GetCurrentMatrix());
		NewtonCollisionCalculateAABB (collision, &matrix[0][0], &boxP0.m_x, &boxP1.m_x); 
		matrix.m_posit = (boxP0 + boxP1).Scale (-0.5f);
		matrix.m_posit.m_w = 1.0f;
		//SetMatrix (matrix);
		entity->ResetMatrix (*scene, matrix);

		// create the terrainBody rigid body
		NewtonBody* const terrainBody = NewtonCreateDynamicBody(scene->GetNewton(), collision, &matrix[0][0]);

		// release the collision tree (this way the application does not have to do book keeping of Newton objects
		NewtonDestroyCollision (collision);

		// in newton 300 collision are instance, you need to ready it after you create a body, if you wnat to male call on teh instance
		collision = NewtonBodyGetCollision(terrainBody);


		// save the pointer to the graphic object with the body.
		NewtonBodySetUserData (terrainBody, entity);

		// set the global position of this body
		//	NewtonBodySetMatrix (m_terrainBody, &matrix[0][0]); 

		// set the destructor for this object
		//NewtonBodySetDestructorCallback (terrainBody, Destructor);

		// get the position of the aabb of this geometry
		//NewtonCollisionCalculateAABB (collision, &matrix[0][0], &boxP0.m_x, &boxP1.m_x); 

#ifdef USE_TEST_ALL_FACE_USER_RAYCAST_CALLBACK
		// set a ray cast callback for all face ray cast 
		NewtonTreeCollisionSetUserRayCastCallback (collision, AllRayHitCallback);
		dVector p0 (0,  100, 0, 0);
		dVector p1 (0, -100, 0, 0);
		dVector normal;
		int id;
		dFloat parameter;
		parameter = NewtonCollisionRayCast (collision, &p0[0], &p1[0], &normal[0], &id);
#endif

		delete[] attibutes;
		delete[] elevation;
		return terrainBody;
	}
};

	

NewtonBody* CreateHeightFieldTerrain (DemoEntityManager* const scene, int sizeInPowerOfTwos, dFloat cellSize, dFloat elevationScale, dFloat roughness, dFloat maxElevation, dFloat minElevation)
{
	return HeightfieldMap::CreateHeightFieldTerrain (scene, sizeInPowerOfTwos, cellSize, elevationScale, roughness, maxElevation, minElevation);
}