// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "VoxelWorld.h"
#include "VoxelMovementComponent.generated.h"

/*
 * 
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class VOXELENGINE_API UVoxelMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VoxelMovement)
	float MaxSpeed;

	/** Acceleration applied by input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VoxelMovement)
	float Acceleration;

	/** Deceleration applied when there is no input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VoxelMovement)
	float Deceleration;

	/**
	 * Setting affecting extra force applied when changing direction, making turns have less drift and become more responsive.
	 * Velocity magnitude is not allowed to increase, that only happens due to normal acceleration. It may decrease with large direction changes.
	 * Larger values apply extra force to reach the target direction more quickly, while a zero value disables any extra turn force.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VoxelMovement, meta = (ClampMin = "0", UIMin = "0"))
	float TurningBoost;

	virtual float GetMaxSpeed() const override { return MaxSpeed; }
protected:
	virtual bool ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotation) override;
private:
	UPROPERTY()
	AVoxelWorld* VoxelWorld;

	UPROPERTY(Transient)
	bool bPositionCorrected = true;

	UPROPERTY(EditAnywhere)
	float GravityMultiplier = 1;

	UPROPERTY(EditAnywhere)
	bool bCheckForUnrealEngineCollisions = false;

	UPROPERTY(EditAnywhere)
	bool bDebugDrawVoxelCollider = false;

	UPROPERTY(VisibleAnywhere)
	bool bIsGrounded = false;

	bool LimitWorldBounds();
	void ApplyControlInputToVelocity(float DeltaTime);
	bool ClampVector(FVector& Vec, const FVector& Min, const FVector& Max) const;
	void ProcessMovement(float DeltaTime, const FBox& VoxelColliderBox, const FVector& DeltaNormalized, double DeltaMagnitude);
	FVector ProcessVoxelCollision(float DeltaTime, const FBox& VoxelColliderBox, const FVector& DeltaNormalized, double DeltaMagnitude);
	bool CheckGround(float DeltaTime, const FBox& VoxelColliderBox, const FVector& DeltaNormalized, double DeltaMagnitude);
	FBox GetVoxelCollider() const;
}; 
