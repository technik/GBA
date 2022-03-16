//
// iwram graphics code
//
#pragma once

#include <cstdint>
#include <linearMath.h>
#include <vector.h>

void PostCameraState(
	math::Vec3p8 Pos,
	math::intp8 Cosf,
	math::intp8 Sinf);
void m7_hbl_c();
void setBg2AffineTx(uint16_t vCount);
