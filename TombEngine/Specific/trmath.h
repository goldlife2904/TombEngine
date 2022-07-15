#pragma once
#include "Specific/phd_global.h"

constexpr auto PI = 3.14159265358979323846f;
constexpr auto RADIAN = 0.01745329252f;
constexpr auto ONE_DEGREE = 182;
constexpr auto PREDICTIVE_SCALE_FACTOR = 14;
constexpr auto WALL_SIZE = 1024;
constexpr auto WALL_MASK = WALL_SIZE - 1;
constexpr auto STEP_SIZE = WALL_SIZE / 4;
constexpr auto STOP_SIZE = WALL_SIZE / 2;
constexpr auto GRID_SNAP_SIZE = STEP_SIZE / 2;
constexpr auto STEPUP_HEIGHT = ((STEP_SIZE * 3) / 2);
constexpr auto SWIM_DEPTH = 730;
constexpr auto WADE_DEPTH = STEPUP_HEIGHT;
constexpr auto SHALLOW_WATER_START_LEVEL = STEP_SIZE / 4;
constexpr auto BAD_JUMP_CEILING = ((STEP_SIZE * 3) / 4);
constexpr auto SLOPE_DIFFERENCE = 60;
constexpr auto NO_HEIGHT  = INT_MIN + UCHAR_MAX;
constexpr auto MAX_HEIGHT = INT_MIN + 1; // Add 1 to prevent issue with sign changes
constexpr auto DEEP_WATER = INT_MAX - 1; // Subtract 1 to prevent issue with sign changes

constexpr auto SQUARE = [](auto x) { return x * x; };
constexpr auto CLICK = [](auto x) { return STEP_SIZE * x; };
constexpr auto SECTOR = [](auto x) { return WALL_SIZE * x; };
constexpr auto MESH_BITS = [](auto x) { return 1 << x; };
constexpr auto OFFSET_RADIUS = [](auto x) { return round(x * M_SQRT2 + 4); };

BoundingOrientedBox TO_DX_BBOX(PHD_3DPOS pos, BOUNDING_BOX* box);

const Vector3 GetRandomVector();
const Vector3 GetRandomVectorInCone(const Vector3& direction,const float angleDegrees);
EulerAngles GetVectorAngles(int x, int y, int z);
EulerAngles GetOrientBetweenPoints(Vector3Int origin, Vector3Int target);
void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, BOUNDING_BOX* bounds, BOUNDING_BOX* tbounds);
int phd_Distance(PHD_3DPOS* first, PHD_3DPOS* second);

void InterpolateAngle(short angle, short* rotation, short* outAngle, int shift);

constexpr auto FP_SHIFT = 16;
constexpr auto FP_ONE = (1 << FP_SHIFT);
constexpr auto W2V_SHIFT = 14;

void FP_VectorMul(Vector3Int* v, int scale, Vector3Int* result);
__int64 FP_Mul(__int64 a, __int64 b);
__int64 FP_Div(__int64 a, __int64 b);
int FP_DotProduct(Vector3Int* a, Vector3Int* b);
void FP_CrossProduct(Vector3Int* a, Vector3Int* b, Vector3Int* n);
void FP_GetMatrixAngles(MATRIX3D* m, short* angles);
__int64 FP_ToFixed(__int64 value);
__int64 FP_FromFixed(__int64 value);
Vector3Int* FP_Normalise(Vector3Int* v);

#define	MULFP(a,b)		(int)((((__int64)a*(__int64)b))>>16)
#define DIVFP(a,b)		(int)(((a)/(b>>8))<<8)

const float Lerp(float v0, float v1, float t);
const float Smoothstep(float edge0, float edge1, float x);

Vector3 TranslateVector(Vector3 vector, float angle, float forward, float up = 0.0f, float right = 0.0f);
Vector3Int TranslateVector(Vector3Int vector, float angle, float forward, float up = 0.0f, float right = 0.0f);
Vector3 TranslateVector(Vector3 vector, EulerAngles orient, float distance);
Vector3Int TranslateVector(Vector3Int vector, EulerAngles orient, float distance);
Vector3 TranslateVector(Vector3 vector, Vector3 target, float distance);
Vector3Int TranslateVector(Vector3Int vector, Vector3Int target, float distance);
