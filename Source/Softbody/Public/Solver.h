#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoftbodyObject.h"
#include "Solver.generated.h"

UCLASS()
class SOFTBODY_API ASolver : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere)
	TArray<ASoftbodyObject*> Bodies;

	UPROPERTY(EditAnywhere)
	int32 NumSubsteps;
	UPROPERTY(EditAnywhere)
	FVector Gravity;

	ASolver();

protected:
	FBodyInstance* BodyInstance;

	// Custom physics Delegate
	FCalculateCustomPhysics OnCalculateCustomPhysics;
	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);

public:
	virtual void Tick(float DeltaTime) override;

};
