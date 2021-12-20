#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGrabber.generated.h"

UCLASS()
class SOFTBODY_API AAGrabber : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class APlayerController* Controller;
	float Distance;
	class ASoftbody* Target;
	FVector PrevPos;
	AAGrabber();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
};
