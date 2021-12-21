#include "Grabber.h"
#include "SoftbodyObject.h"

AGrabber::AGrabber()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGrabber::BeginPlay()
{
	Super::BeginPlay();	
	Target = nullptr;
	Controller = GetWorld()->GetFirstPlayerController();
	if (Controller)
	{
		Controller->bShowMouseCursor = true;
	}
}

void AGrabber::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("Grabber could not find a player controller"));
		return;
	}

	FVector WorldOrigin;
	FVector WorldDirection;
	if (Controller->DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		if (Controller->WasInputKeyJustPressed(EKeys::LeftMouseButton))
		{
			FHitResult hit;
			FCollisionQueryParams CollisionQuery = FCollisionQueryParams::DefaultQueryParam;
			CollisionQuery.bReturnFaceIndex = true;
			if (GetWorld()->LineTraceSingleByChannel(hit, WorldOrigin, WorldOrigin + WorldDirection * 100000, ECollisionChannel::ECC_Visibility, CollisionQuery))
			{
				if (ASoftbodyObject* Body = Cast<ASoftbodyObject>(hit.GetActor()))
				{
					Distance = hit.Distance;
					PrevPos = WorldOrigin + WorldDirection * Distance;
					Target = Body;
					Target->StartGrab(hit.ImpactPoint, hit.FaceIndex);
				}
			}
		}

		if (Target)
		{
			FVector Pos = WorldOrigin + WorldDirection * Distance;
			Target->MovedGrabbed(Pos);
			if (Controller->WasInputKeyJustReleased(EKeys::LeftMouseButton))
			{
				Target->EndGrab(PrevPos - Pos);
				Target = nullptr;
			}
			PrevPos = Pos;
		}

	}
}

