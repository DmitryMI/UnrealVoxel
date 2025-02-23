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

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FBox VoxelColliderBox = GetVoxelCollider();

	if (Controller->IsLocalPlayerController() == true || Controller->IsFollowingAPath() == false || NavMovementProperties.bUseAccelerationForPaths)
	{
		ApplyControlInputToVelocity(DeltaTime);
	}
	else if (IsExceedingMaxSpeed(MaxSpeed) == true)
	{
		Velocity = Velocity.GetUnsafeNormal() * MaxSpeed;
	}

	bPositionCorrected = false;
	bIsGrounded = CheckGround(DeltaTime, VoxelColliderBox);
	if (!bIsGrounded)
	{
		Velocity += FVector::UpVector * GetGravityZ() * GravityMultiplier * DeltaTime;
	}
	
	ProcessMovement(DeltaTime);

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

void UVoxelMovementComponent::ProcessMovement(float DeltaTime)
{
	UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent);

	FVector Delta = Velocity * DeltaTime;

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FQuat Rotation = UpdatedComponent->GetComponentQuat();

	if (!RootPrimitive)
	{
		MoveUpdatedComponent(Delta, Rotation, false);
		return;
	}
	
	float CyllinderRadius;
	float CyllinderHalfHeight;
	RootPrimitive->CalcBoundingCylinder(CyllinderRadius, CyllinderHalfHeight);
	float CyllinderHeight = CyllinderHalfHeight * 2;
	FVector Extent(CyllinderRadius, CyllinderRadius, CyllinderHalfHeight);
	FBox VoxelColliderBox = FBox::BuildAABB(RootPrimitive->GetComponentLocation(), Extent);
	Delta = ProcessVoxelCollision(DeltaTime, VoxelColliderBox);

	MoveUpdatedComponent(Delta, Rotation, false);

	// Update velocity
	// We don't want position changes to vastly reverse our direction (which can happen due to penetration fixups etc)
	if (bPositionCorrected)
	{
		const FVector NewLocation = UpdatedComponent->GetComponentLocation();
		Velocity = ((NewLocation - OldLocation) / DeltaTime);
	}
}

FVector UVoxelMovementComponent::ProcessVoxelCollision(float DeltaTime, const FBox& VoxelCollider)
{
	FVector Delta = Velocity * DeltaTime;
	FVector MovementDirection = Delta.GetUnsafeNormal();
	FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	FVector ProjectedLocation = CurrentLocation + Delta;
	FBox ProjectedCollider = VoxelCollider.MoveTo(ProjectedLocation);

	FVoxelQueryFilterParams Filter;
	Filter.Traversible = EVoxelLineTraceFilterMode::Negative;
	TArray<FIntVector> OverlappedVoxels;
	bool bHasCollisions = UVoxelQueryUtils::VoxelBoxOverlapFilterMulti(VoxelWorld, ProjectedCollider, OverlappedVoxels, Filter);
	if (!bHasCollisions)
	{
		return Delta;
	}

	for (const FIntVector& OverlappedVoxel : OverlappedVoxels)
	{
		FBox OverlappedVoxelBox = VoxelWorld->GetVoxelBoundingBox(OverlappedVoxel);
		FBox Overlap = OverlappedVoxelBox.Overlap(ProjectedCollider);
		if (Overlap.GetSize().IsNearlyZero(0.1))
		{
			continue;
		}

		DrawDebugBox(GetWorld(), Overlap.GetCenter(), Overlap.GetExtent(), FColor::Red, false, -1, 10, 0);

		FVector AxisDistances = (OverlappedVoxelBox.GetCenter() - ProjectedCollider.GetCenter()).GetAbs() - (OverlappedVoxelBox.GetExtent() + ProjectedCollider.GetExtent());

		FVector PenetrationPull = MovementDirection * FVector::DotProduct(MovementDirection, AxisDistances);
		DrawDebugLine(GetWorld(), ProjectedLocation, ProjectedLocation - PenetrationPull, FColor::Blue);

		ProjectedLocation -= PenetrationPull;
		ProjectedCollider = ProjectedCollider.MoveTo(ProjectedLocation);
		bPositionCorrected = true;
	}

	Delta = ProjectedLocation - CurrentLocation;
	return Delta;
}

bool UVoxelMovementComponent::CheckGround(float DeltaTime, const FBox& VoxelColliderBox)
{
	if (Velocity.Z > 0)
	{
		return false;
	}

	FVector Delta = Velocity * DeltaTime;
	double DeltaMagnitude = Delta.Size();
	FVector MovementDirection = Delta / DeltaMagnitude;
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
		FVector SweepDelta = MovementDirection * SweepDeltaMagnitude;
		
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
