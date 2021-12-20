#include "ASoftbody.h"
#include "ProceduralMeshComponent.h"

ASoftbody::ASoftbody()
{
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("proceduralMesh");
	SetRootComponent(ProceduralMesh);
	ProceduralMesh->bUseAsyncCooking = true;
}

void ASoftbody::BeginPlay()
{
	Super::BeginPlay();

	if (!TetMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Tet Mesh data assigned"));
		NumParticles = 0;
		return;
	}

	FTetMesh* tetMeshData = TetMesh->FindRow<FTetMesh>(TetMesh->GetRowNames()[0], TEXT("GENERAL"));
	CreateProceduralMesh(tetMeshData);
	
	//Init Physics
	Vel.Init(FVector::ZeroVector, NumParticles);
	PrevPos = TArray<FVector>(Pos);

	InvMass.Init(1, NumParticles);
	TetIds = TArray<int>(tetMeshData->tetIds);
	TriangleIDs = tetMeshData->tetSurfaceTriIds;
	EdgeIds = TArray<int>(tetMeshData->tetEdgeIds);

	NumTets = tetMeshData->tetIds.Num() / 4;
	RestVol.Init(0.0, NumTets);
	EdgeLengths.Init(0.0, EdgeIds.Num() / 2);

	Temp.Init(FVector::ZeroVector, NumParticles);
	Grads.Init(FVector::ZeroVector, NumParticles);

	GrabId = -1;
	GrabInvMass = 0.0f;
	
	for (int i = 0; i < NumTets; i++)
	{
		float vol = GetTetVolume(i);
		RestVol[i] = vol;
		float pInvMass = vol > 0.0f ? 1.0 / (vol / 4.0) : 0.0;
		InvMass[TetIds[4 * i]] += pInvMass;
		InvMass[TetIds[4 * i + 1]] += pInvMass;
		InvMass[TetIds[4 * i + 2]] += pInvMass;
		InvMass[TetIds[4 * i + 3]] += pInvMass;
	}

	for (int i = 0; i < EdgeLengths.Num(); i++)
	{
		int id0 = EdgeIds[2 * i];
		int id1 = EdgeIds[2 * i + 1];
		EdgeLengths[i] = FMath::Sqrt((Pos[id0] - Pos[id1]).SizeSquared());
	}
}

void ASoftbody::CalculateNormals()
{
	for (int i = 0; i < Triangles.Num(); i += 3)
	{
		int Index1 = Triangles[i];
		int Index2 = Triangles[i + 1];
		int Index3 = Triangles[i + 2];

		FVector P1 = Pos[Index1];
		FVector P2 = Pos[Index2];
		FVector P3 = Pos[Index3];

		FVector normal = FVector::CrossProduct(P2 - P1, P3 - P1) * -1; //TODO: why do I need to flip? Is this wrong?
		normal.Normalize();

		Normals[Index1] = normal;
		Normals[Index2] = normal;
		Normals[Index3] = normal;
	}
}

float ASoftbody::GetTetVolume(int Nr)
{
	int Id0 = TetIds[4 * Nr];
	int Id1 = TetIds[4 * Nr + 1];
	int Id2 = TetIds[4 * Nr + 2];
	int Id3 = TetIds[4 * Nr + 3];

	Temp[0] = Pos[Id1] - Pos[Id0];
	Temp[1] = Pos[Id2] - Pos[Id0];
	Temp[2] = Pos[Id3] - Pos[Id0];
	Temp[3] = FVector::CrossProduct(Temp[0], Temp[1]);
	return FVector::DotProduct(Temp[3], Temp[2]) / 6.0;
}

void ASoftbody::PreSolve(float DeltaTime, FVector Gravity)
{
	for (int i = 0; i < NumParticles; i++)
	{
		if (InvMass[i] == 0.0)
		{
			continue;
		}

		Vel[i] += Gravity * DeltaTime;
		PrevPos[i] = Pos[i];
		Pos[i] += Vel[i] * DeltaTime;

		//Ground collision
		FVector GroundPosition = FVector::ZeroVector;
		GroundPosition = GetTransform().InverseTransformPosition(GroundPosition);
		if (Pos[i].Z < GroundPosition.Z)
		{
			Pos[i] = PrevPos[i];
			Pos[i].Z = GroundPosition.Z;
		}
	}
}

void ASoftbody::Solve(float DeltaTime)
{
	SolveEdges(EdgeCompliance, DeltaTime);
	SolveVolumes(VolCompliance, DeltaTime);
}

void ASoftbody::PostSolve(float DeltaTime)
{
	for (int i = 0; i < NumParticles; i++)
	{
		if (InvMass[i] == 0.0)
		{
			continue;
		}
		Vel[i] = (Pos[i] - PrevPos[i]) * (1.0 / DeltaTime);
	}
}

void ASoftbody::SolveEdges(float Compliance, float DeltaTime)
{
	float alpha = Compliance / DeltaTime / DeltaTime;

	for (int i = 0; i < EdgeLengths.Num(); i++)
	{
		int id0 = EdgeIds[2 * i];
		int id1 = EdgeIds[2 * i + 1];
		float w0 = InvMass[id0];
		float w1 = InvMass[id1];
		float w = w0 + w1;
		if (w == 0.0)
			continue;

		Grads[0] = Pos[id0] - Pos[id1];
		float len = FMath::Sqrt(Grads[0].SizeSquared());
		if (len == 0.0f)
		{
			continue;
		}

		Grads[0] *= (1.0f / len);
		float restLen = EdgeLengths[i];
		float C = len - restLen;
		float s = -C / (w + alpha);
		Pos[id0] += Grads[0] * (s * w0);
		Pos[id1] += Grads[0] * (-s * w1);
	}
}

void ASoftbody::SolveVolumes(float Compliance, float DeltaTime)
{
	float alpha = Compliance / DeltaTime / DeltaTime;
	for (int i = 0; i < NumTets; i++)
	{
		float w = 0.0;

		for (int j = 0; j < 4; j++)
		{
			int id0 = TetIds[4 * i + VolIdOrder[j][0]];
			int id1 = TetIds[4 * i + VolIdOrder[j][1]];
			int id2 = TetIds[4 * i + VolIdOrder[j][2]];

			Temp[0] = Pos[id1] - Pos[id0];
			Temp[1] = Pos[id2] - Pos[id0];
			Grads[j] = FVector::CrossProduct(Temp[0], Temp[1]);
			Grads[j] *= (1.0f / 6.0f);

			w += InvMass[TetIds[4 * i + j]] * Grads[j].SizeSquared();
		}
		if (w == 0.0)
			continue;

		float vol = GetTetVolume(i);
		float restV = RestVol[i];
		float C = vol - restV;
		float s = -C / (w + alpha);

		for (int j = 0; j < 4; j++)
		{
			int id = TetIds[4 * i + j];
			Pos[id] += Grads[j] * (s * InvMass[id]);
		}
	}
}

void ASoftbody::CreateProceduralMesh(FTetMesh* TetMeshData)
{
	NumParticles = TetMeshData->verts.Num() / 3;
	Pos.Init(FVector::ZeroVector, NumParticles);
	for (int i = 0; i < TetMeshData->verts.Num(); i += 3)
	{
		FVector P = FVector(TetMeshData->verts[i], TetMeshData->verts[i + 1], TetMeshData->verts[i + 2]);
		//Convert to Unreal coordinates
		Pos[i / 3] = FVector(P.Z, P.X, P.Y);
	}

	//Create Mesh
	Triangles = TArray<int32>(TetMeshData->tetSurfaceTriIds);
	for (int i = 0; i < Triangles.Num(); i += 3)
	{
		//Convert winding order
		int TempId = Triangles[i];
		Triangles[i] = Triangles[i + 2];
		Triangles[i + 2] = TempId;
	}

	Normals.Init(FVector::ZeroVector, NumParticles);
	CalculateNormals();

	ProceduralMesh->CreateMeshSection_LinearColor(0, Pos, Triangles, Normals, TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
	ProceduralMesh->ContainsPhysicsTriMeshData(true);
	ProceduralMesh->SetMaterial(0, Material);
}

void ASoftbody::StartGrab(FVector Point, int FaceID)
{
	Point = GetTransform().InverseTransformPosition(Point);

	//Get closest point id in triangle
	float Closest = TNumericLimits<float>::Max();
	for (int i = FaceID * 3; i < FaceID * 3 + 3; i++)
	{
		FVector P = Pos[TriangleIDs[i]];
		float Distance = (P - Point).SizeSquared();
		if (Distance < Closest)
		{
			GrabId = TriangleIDs[i];
			Closest = Distance;
		}
	}
	GrabInvMass = InvMass[GrabId];
	InvMass[GrabId] = 0.0f;
	Pos[GrabId] = Point;
}

void ASoftbody::MovedGrabbed(FVector GrabPos)
{
	if (GrabId >= 0)
	{
		GrabPos = GetTransform().InverseTransformPosition(GrabPos);
		Pos[GrabId] = GrabPos;
	}
}

void ASoftbody::EndGrab(FVector GrabVel)
{
	InvMass[GrabId] = GrabInvMass;
	Vel[GrabId] = GrabVel;
	GrabId = -1;
}

void ASoftbody::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CalculateNormals();
	ProceduralMesh->UpdateMeshSection_LinearColor(0, Pos, Normals, TArray<FVector2D>(), TArray<FLinearColor>(), TArray<FProcMeshTangent>());
}

void ASoftbody::OnConstruction(const FTransform& Transform)
{
	FTetMesh* tetMeshData = TetMesh->FindRow<FTetMesh>(TetMesh->GetRowNames()[0], TEXT("GENERAL"));
	CreateProceduralMesh(tetMeshData);
}

