#pragma once

struct MoveTestSetup
{
	short Angle;
	int LowerBound;
	int UpperBound;
	bool CheckSlopeDown = true;
	bool CheckSlopeUp = true;
	bool CheckDeath = true;
};

struct VaultTestSetup
{
	int LowerBound;
	int UpperBound;
	int ClampMin;
	int ClampMax;
	int GapMin;
	bool CheckSwampDepth = true;
};

struct VaultTestResult
{
	bool Success;
	int Height;
};

struct CrawlVaultTestSetup
{
	int LowerBound;
	int UpperBound;
	int ClampMin;
	int GapMin;
	int CrossDist;
	int DestDist;
	int ProbeHeightDifMax;
	bool CheckSlope = true;
	bool CheckDeath = true;
};

struct CornerTestResult
{
	bool Success;
	PHD_3DPOS ProbeResult;
	PHD_3DPOS RealPositionResult;
};
