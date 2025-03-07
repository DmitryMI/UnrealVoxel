// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "VoxelQueryUtils.h"
#include "VoxelEngine/VoxelEngine.h"

void UVoxelMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	// TODO Very slow! Replace with dependency injection.
	VoxelWorld = Cast<AVoxelWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AVoxelWorld::StaticClass()));
}

bool UVoxelMovementComponent::ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotation)
{
	bPositionCorrected |= Super::ResolvePenetrationImpl(Adjustment, Hit, NewRotation);
	return bPositionCorrected;
}

bool UVoxelMovementComponent::IsExceedingMaxSpeed(float MaxSpeedArg) const
{
	MaxSpeedArg = FMath::Max(0.f, MaxSpeedArg);
	const float MaxSpeedSquared = FMath::Square(MaxSpeedArg);

	// Allow 1% error tolerance, to account for numeric imprecision.
	const float OverVelocityPercent = 1.01f;
	return (Velocity.SizeSquared2D() > MaxSpeedSquared * OverVelocityPercent);
}

void UVoxelMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent || !VoxelWorld)
	{
		return;
	}

	const AController* Controller = PawnOwner->GetController();
	if (!Controller || !Controller->IsLocalController())
	{
		return;
	}

	FVector PendingInputVector = GetPendingInputVector();
	ConsumeInputVector();
	bool bWantsJumping = PendingInputVector.Z > 0;
	PendingInputVector.Z = 0;
	PendingInputVector = PendingInputVector.GetClampedToMaxSize(1.0);

	if (Controller->IsLocalPlayerController() == true || Controller->IsFollowingAPath() == false || NavMovementProperties.bUseAccelerationForPaths)
	{
		ApplyControlInputToVelocity(DeltaTime, PendingInputVector);
	}
	else if (IsExceedingMaxSpeed(MaxSpeed) == true)
	{
		Velocity = Velocity.GetUnsafeNormal() * MaxSpeed;
	}

	Velocity += FVector::UpVector * GetGravityZ() * GravityMultiplier * DeltaTime;

	if (bIsGrounded && bWantsJumping)
	{
		Velocity += FVector::UpVector * JumpVelocity;
	}

	const FQuat Rotation = UpdatedComponent->GetComponentQuat();
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FBox VoxelColliderBox = GetVoxelCollider();
	FVector Delta = Velocity * DeltaTime;
	
	if (!VoxelColliderBox.IsValid)
	{
		MoveUpdatedComponent(Delta, Rotation, false);
		return;
	}

	bPositionCorrected = false;
	
	FIntVector OutDirectionBlocked{ 0, 0, 0 };
	ProcessVoxelCollision(DeltaTime, VoxelColliderBox, Delta, OutDirectionBlocked);
	bIsGrounded = OutDirectionBlocked.Z < 0;

	MoveUpdatedComponent(Delta, Rotation, false);

	LimitWorldBounds();
	
	FVector NewLocation = UpdatedComponent->GetComponentLocation();
	Velocity = (NewLocation - OldLocation) / DeltaTime;

	UpdateComponentVelocity();
}

bool UVoxelMovementComponent::LimitWorldBounds()
{
	FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	FBox WorldBox = VoxelWorld->GetBoundingBoxWorld();
	if (ClampVector(CurrentLocation, WorldBox.Min, WorldBox.Max))
	{
		UpdatedComponent->SetRelativeLocation(CurrentLocation);
		return true;
	}
	
	return false;
}

void UVoxelMovementComponent::ApplyControlInputToVelocity(float DeltaTime, FVector& PendingInputVector)
{
	const float AnalogInputModifier = (PendingInputVector.SizeSquared() > 0.f ? PendingInputVector.Size() : 0.f);
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
	const bool BExceedingMaxSpeed = IsExceedingMaxSpeed(MaxPawnSpeed);
	const FVector OldVelocity = Velocity;

	if (AnalogInputModifier > 0.f && !BExceedingMaxSpeed)
	{
		// Apply change in velocity direction
		if (Velocity.SizeSquared2D() > 0.f)
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = FMath::Clamp(DeltaTime * TurningBoost, 0.f, 1.f);
			Velocity = Velocity + (PendingInputVector * Velocity.Size2D() - Velocity) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if (Velocity.SizeSquared2D() > 0.f)
		{
			
			const float VelSize = FMath::Max(Velocity.Size2D() - FMath::Abs(Deceleration) * DeltaTime, 0.f);
			Velocity = Velocity.GetSafeNormal() * VelSize;
			// Don't allow braking to lower us below max speed if we started above it.
			if (BExceedingMaxSpeed && Velocity.SizeSquared2D() < FMath::Square(MaxPawnSpeed))
			{
				Velocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = (IsExceedingMaxSpeed(MaxPawnSpeed)) ? Velocity.Size2D() : MaxPawnSpeed;
	Velocity += PendingInputVector * FMath::Abs(Acceleration) * DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize2D(NewMaxSpeed);
	Velocity.Z = OldVelocity.Z;
}

bool UVoxelMovementComponent::ClampVector(FVector& Vec, const FVector& Min, const FVector& Max) const
{
	bool bWasChanged = false;
	
	for (int I = 0; I < 3; I++)
	{
		if (Vec[I] < Min[I])
		{
			Vec[I] = Min[I];
			bWasChanged = true;
		}
		else if (Vec[I] > Max[I])
		{
			Vec[I] = Max[I];
			bWasChanged = true;
		}
	}

	return bWasChanged;
}

void UVoxelMovementComponent::ProcessVoxelCollision(float DeltaTime, const FBox& VoxelColliderBox, FVector& InOutDelta, FIntVector& OutDirectionBlocked)
{
	FVector DeltaAbs = InOutDelta.GetAbs();
	
	double SweepStep = VoxelWorld->GetVoxelSizeWorld() / 3;
	
	for (int Dim = 0; Dim < 3; Dim++)
	{
		int SweepStepsNum = FMath::CeilToInt(DeltaAbs[Dim] / SweepStep);

		for (int SweepIndex = 0; SweepIndex < SweepStepsNum; SweepIndex++)
		{
			FVector SweepDelta{ 0, 0, 0 };
			SweepDelta[Dim] = FMath::Sign(InOutDelta[Dim]) * SweepStep * (SweepIndex + 1);
			if (FMath::Abs(SweepDelta[Dim]) > DeltaAbs[Dim])
			{
				SweepDelta[Dim] = InOutDelta[Dim];
			}
			FBox ColliderSweep = VoxelColliderBox.ShiftBy(SweepDelta);
			FVector SweepCenter = ColliderSweep.GetCenter();
			TArray<FIntVector> BlockingVoxels;
			FVoxelQueryFilterParams Filter;
			Filter.Traversible = EVoxelLineTraceFilterMode::Negative;
			bool bHasHit = UVoxelQueryUtils::VoxelBoxOverlapFilterMulti(VoxelWorld, ColliderSweep, BlockingVoxels, Filter);

			if (!bHasHit)
			{
				continue;
			}

			bool bVelocityClamped = false;

			for (const FIntVector& Voxel : BlockingVoxels)
			{
				FBox VoxelBox = VoxelWorld->GetVoxelBoundingBox(Voxel);
				FBox Overlap = VoxelBox.Overlap(ColliderSweep);
				if (!Overlap.IsValid || FMath::IsNearlyZero(Overlap.GetVolume()))
				{
					continue;
				}
				FVector VoxelCenter = VoxelBox.GetCenter();
				FVector DirectionToVoxel = VoxelCenter - SweepCenter;
				if (FMath::Sign(DirectionToVoxel[Dim]) != FMath::Sign(InOutDelta[Dim]))
				{
					continue;
				}
				FVector DistanceBetweenBoxes = (VoxelColliderBox.GetCenter() - VoxelBox.GetCenter()).GetAbs() - (VoxelColliderBox.GetExtent() + VoxelBox.GetExtent());
				double DistanceToSurface = DistanceBetweenBoxes[Dim];
				if (DistanceToSurface < 0)
				{
					continue;
				}
				if (FMath::IsNearlyZero(DistanceToSurface, 0.01) || DistanceToSurface < DeltaAbs[Dim])
				{
					int SpeedSign = FMath::Sign(Velocity[Dim]);
					double Speed = DistanceToSurface / DeltaTime;
					if (FMath::Abs(Velocity[Dim]) > Speed)
					{
						Velocity[Dim] = SpeedSign * Speed;
						InOutDelta[Dim] = SpeedSign * DistanceToSurface;
						DeltaAbs[Dim] = DistanceToSurface;
						OutDirectionBlocked[Dim] = SpeedSign;
					}
					bVelocityClamped = true;
				}
			}

			if (bVelocityClamped)
			{
				break;
			}
		}
	}
}

FBox UVoxelMovementComponent::GetVoxelCollider() const
{
	UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent);
	if (!RootPrimitive)
	{
		return FBox(EForceInit::ForceInit);
	}
	float CyllinderRadius;
	float CyllinderHalfHeight;
	RootPrimitive->CalcBoundingCylinder(CyllinderRadius, CyllinderHalfHeight);
	float CyllinderHeight = CyllinderHalfHeight * 2;
	FVector VoxelColliderBoxExtent(CyllinderRadius, CyllinderRadius, CyllinderHalfHeight);
	FBox VoxelColliderBox = FBox::BuildAABB(RootPrimitive->GetComponentLocation(), VoxelColliderBoxExtent);
	return VoxelColliderBox;
}
