#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "ProceduralMeshComponent.h"
#include "ASoftbody.generated.h"

UCLASS()
class SOFTBODY_API ASoftbody : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	class UDataTable* TetMesh;

	UPROPERTY(EditAnywhere)
	class UMaterialInterface* Material;

	UPROPERTY(EditAnywhere)
	float EdgeCompliance = 100.0;

	UPROPERTY(EditAnywhere)
	float VolCompliance = 0.0;

	ASoftbody();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UProceduralMeshComponent* ProceduralMesh;

	int NumParticles;
	TArray<FVector> Pos;
	TArray<FVector> Vel;
	TArray<FVector> PrevPos;
	TArray<FVector> Normals;
	TArray<int32> Triangles;
	TArray<float> InvMass;
	TArray<int> TetIds;
	TArray<int> EdgeIds;
	TArray<int> TriangleIDs;
	TArray<float> RestVol;
	TArray<float> EdgeLengths;
	TArray<FVector> Temp;
	TArray<FVector> Grads;
	int GrabId;
	float GrabInvMass;
	int NumTets;
	int VolIdOrder[4][3] =
	{
		{1, 3, 2},
		{0, 2, 3},
		{0, 3, 1},
		{0, 1, 2}
	};

	virtual void BeginPlay() override;
	void CalculateNormals();
	float GetTetVolume(int Nr);
	void SolveEdges(float Compliance, float Dt);
	void SolveVolumes(float Compliance, float Dt);
	void CreateProceduralMesh(FTetMesh* tetMeshData);

public:	
	void PreSolve(float Dt, FVector Gravity);
	void Solve(float Dt);
	void PostSolve(float Dt);
	void StartGrab(FVector GrabPoint, int FaceID);
	void MovedGrabbed(FVector GrabPos);
	void EndGrab(FVector GrabVel);

	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

};

USTRUCT(BlueprintType)
struct FTetMesh : public FTableRowBase
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Softbody")
	FText name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Softbody")
	TArray<float> verts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Softbody")
	TArray<int32> tetIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Softbody")
	TArray<int32> tetEdgeIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Softbody")
	TArray<int32> tetSurfaceTriIds;
};