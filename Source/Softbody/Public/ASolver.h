#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASoftbody.h"
#include "ASolver.generated.h"

UCLASS()
class SOFTBODY_API ASolver : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere)
	TArray<ASoftbody*> Bodies;

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
