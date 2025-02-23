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

	if (Controller->IsLocalPlayerController() == true || Controller->IsFollowingAPath() == false || NavMovementProperties.bUseAccelerationForPaths)
	{
		ApplyControlInputToVelocity(DeltaTime);
	}
	else if (IsExceedingMaxSpeed(MaxSpeed) == true)
	{
		Velocity = Velocity.GetUnsafeNormal() * MaxSpeed;
	}

	const FQuat Rotation = UpdatedComponent->GetComponentQuat();
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FBox VoxelColliderBox = GetVoxelCollider();
	FVector Delta = Velocity * DeltaTime;
	double DeltaMagnitude = 0;
	FVector DeltaNormalized{0, 0, 0};
	if (!Delta.IsNearlyZero())
	{
		DeltaMagnitude = Delta.Size();
		DeltaNormalized = Delta / DeltaMagnitude;
	}

	if (!bIsGrounded)
	{
		Velocity += FVector::UpVector * GetGravityZ() * GravityMultiplier * DeltaTime;
	}

	if (!VoxelColliderBox.IsValid)
	{
		MoveUpdatedComponent(Delta, Rotation, false);
		return;
	}

	bPositionCorrected = false;
	bIsGrounded = CheckGround(DeltaTime, VoxelColliderBox, DeltaNormalized, DeltaMagnitude);
	
	Delta = ProcessVoxelCollision(DeltaTime, VoxelColliderBox, DeltaNormalized, DeltaMagnitude);

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

void UVoxelMovementComponent::ApplyControlInputToVelocity(float DeltaTime)
{
	const FVector ControlAcceleration = GetPendingInputVector().GetClampedToMaxSize(1.f);

	const float AnalogInputModifier = (ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f);
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
	const bool BExceedingMaxSpeed = IsExceedingMaxSpeed(MaxPawnSpeed);

	if (AnalogInputModifier > 0.f && !BExceedingMaxSpeed)
	{
		// Apply change in velocity direction
		if (Velocity.SizeSquared() > 0.f)
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = FMath::Clamp(DeltaTime * TurningBoost, 0.f, 1.f);
			Velocity = Velocity + (ControlAcceleration * Velocity.Size() - Velocity) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if (Velocity.SizeSquared() > 0.f)
		{
			const FVector OldVelocity = Velocity;
			const float VelSize = FMath::Max(Velocity.Size() - FMath::Abs(Deceleration) * DeltaTime, 0.f);
			Velocity = Velocity.GetSafeNormal() * VelSize;
			// Don't allow braking to lower us below max speed if we started above it.
			if (BExceedingMaxSpeed && Velocity.SizeSquared() < FMath::Square(MaxPawnSpeed))
			{
				Velocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}

			Velocity.Z = OldVelocity.Z;
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = (IsExceedingMaxSpeed(MaxPawnSpeed)) ? Velocity.Size() : MaxPawnSpeed;
	Velocity += ControlAcceleration * FMath::Abs(Acceleration) * DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize(NewMaxSpeed);

	ConsumeInputVector();
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

void UVoxelMovementComponent::ProcessMovement(float DeltaTime, const FBox& VoxelColliderBox, const FVector& DeltaNormalized, double DeltaMagnitude)
{
	
}

FVector UVoxelMovementComponent::ProcessVoxelCollision(float DeltaTime, const FBox& VoxelColliderBox, const FVector& DeltaNormalized, double DeltaMagnitude)
{
	if (FMath::IsNearlyZero(DeltaMagnitude))
	{
		return FVector::Zero();
	}
	FVector Delta = DeltaNormalized * DeltaMagnitude;
	FVector DeltaAbs = Delta.GetAbs();
	
	double SweepStep = VoxelWorld->GetVoxelSizeWorld() / 3;
	
	for (int Dim = 0; Dim < 3; Dim++)
	{
		int SweepStepsNum = FMath::CeilToInt(DeltaAbs[Dim] / SweepStep);

		for (int SweepIndex = 0; SweepIndex < SweepStepsNum; SweepIndex++)
		{
			FVector SweepDelta{ 0, 0, 0 };
			SweepDelta[Dim] = FMath::Sign(Delta[Dim]) * SweepStep * (SweepIndex + 1);
			if (FMath::Abs(SweepDelta[Dim]) > DeltaAbs[Dim])
			{
				SweepDelta[Dim] = Delta[Dim];
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
				if (FMath::Sign(DirectionToVoxel[Dim]) != FMath::Sign(Delta[Dim]))
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
						Delta[Dim] = SpeedSign * Speed;
						DeltaAbs[Dim] = Speed;
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

	return Velocity * DeltaTime;
}

bool UVoxelMovementComponent::CheckGround(float DeltaTime, const FBox& VoxelColliderBox, const FVector& DeltaNormalized, double DeltaMagnitude)
{
	if (Velocity.Z > 0)
	{
		return false;
	}

	FVector FeetLocation = VoxelColliderBox.GetCenter();
	FeetLocation.Z -= VoxelColliderBox.GetExtent().Z;

	double SweepStep = VoxelWorld->GetVoxelSizeWorld() / 3;
	
	int SweepStepsNum = FMath::Max(FMath::RoundToInt(DeltaMagnitude / SweepStep), 1);

	for (int I = 0; I < SweepStepsNum; I++)
	{
		double SweepDeltaMagnitude = SweepStep * I;
		if (SweepDeltaMagnitude > DeltaMagnitude)
		{
			SweepDeltaMagnitude = DeltaMagnitude;
		}
		FVector SweepDelta = DeltaNormalized * SweepDeltaMagnitude;
		
		FVector FeetLocationSweep = FeetLocation + SweepDelta;
		FVector GroundQueryBoxMin = FVector(VoxelColliderBox.Min.X, VoxelColliderBox.Min.Y, FeetLocationSweep.Z - SweepStep);
		FVector GroundQueryBoxMax = FVector(VoxelColliderBox.Max.X, VoxelColliderBox.Max.Y, FeetLocationSweep.Z);
		FBox GroundQueryBox = FBox(GroundQueryBoxMin, GroundQueryBoxMax);

		TArray<FIntVector> GroundVoxels;
		FVoxelQueryFilterParams Filter;
		Filter.Traversible = EVoxelLineTraceFilterMode::Negative;
		bool bHasHit = UVoxelQueryUtils::VoxelBoxOverlapFilterMulti(VoxelWorld, GroundQueryBox, GroundVoxels, Filter);

		if (!bHasHit)
		{
			continue;
		}

		for (const FIntVector& Voxel : GroundVoxels)
		{
			FBox VoxelBox = VoxelWorld->GetVoxelBoundingBox(Voxel);
			double DistanceToSurface = VoxelBox.Max.Z - FeetLocation.Z;
			if (FMath::IsNearlyZero(DistanceToSurface, 0.01) || FMath::Abs(DistanceToSurface) < DeltaMagnitude)
			{
				Velocity.Z = DistanceToSurface / DeltaTime;
				return true;
			}
		}
	}

	return false;
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
