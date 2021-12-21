#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grabber.generated.h"

UCLASS()
class SOFTBODY_API AGrabber : public AActor
{
	GENERATED_BODY()
	
public:	
	AGrabber();

protected:
	virtual void BeginPlay() override;
	float Distance;
	class ASoftbodyObject* Target;
	FVector PrevPos;
	class APlayerController* Controller;

public:	
	virtual void Tick(float DeltaTime) override;
};
