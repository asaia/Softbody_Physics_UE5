#include "Solver.h"
#include "Components/SphereComponent.h"

ASolver::ASolver()
{
	PrimaryActorTick.bCanEverTick = true;
	NumSubsteps = 10;
	Gravity = FVector(0, 0, -10);

	// Creating a root body instance to get the custom physics callback
	USphereComponent* RootPrim = CreateDefaultSubobject<USphereComponent>("RootPrim");
	SetRootComponent(RootPrim);
	RootPrim->SetSimulatePhysics(true);
	RootPrim->SetMobility(EComponentMobility::Static);
	RootPrim->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	BodyInstance = RootPrim->GetBodyInstance();
	OnCalculateCustomPhysics.BindUObject(this, &ASolver::CustomPhysics);
}

void ASolver::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (BodyInstance)
	{
		BodyInstance->AddCustomPhysics(OnCalculateCustomPhysics);
	}
}

void ASolver::CustomPhysics(float DeltaTime, FBodyInstance* Body)
{
	float SubDeltaTime = DeltaTime / (float)NumSubsteps;
	for (int step = 0; step < NumSubsteps; step++)
	{
		for (int i = 0; i < Bodies.Num(); i++)
		{
			Bodies[i]->PreSolve(SubDeltaTime, Gravity);
		}

		for (int i = 0; i < Bodies.Num(); i++)
		{
			Bodies[i]->Solve(SubDeltaTime);
		}

		for (int i = 0; i < Bodies.Num(); i++)
		{
			Bodies[i]->PostSolve(SubDeltaTime);
		}
	}
}

